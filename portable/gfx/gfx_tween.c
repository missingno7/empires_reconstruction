/* gfx_tween.c -- frame interpolation for the VGA presenter; see gfx_tween.h.
 *
 * Threading: capture and publish run on the game thread; compose runs on
 * the presenter thread.  The only shared object is a small ring of
 * published frames, handed over under a mutex by value (a few hundred KB,
 * ten times a second); the presenter keeps its own per-object history, so
 * composition never holds the lock.
 *
 * Presenter model: every tagged draw is an "op".  The presenter keeps, per
 * (tag, ordinal-within-publish), the latest op and the one before it, each
 * with the publish time and deadline of the frame that carried it.  An op
 * is active while its own window [publish, deadline) is running or when it
 * belongs to the newest frame; active ops are erased (their covered pixels
 * put back, newest first) and redrawn in order at lerp(previous, latest,
 * alpha).  That handles frame loops (every object redrawn every frame), the
 * intro's event-driven single sprite (windows of varying length, erase
 * events in between) and objects that skip a frame, with one rule.
 *
 * The flashlight beam is the one special kind: 24 trail pixels of which 8
 * are new each frame and 8 drop off the tail.  Tagged by age, the presenter
 * grows the head and shortens the tail progressively instead of lerping.
 */
#include "gfx_tween.h"
#include "gfx.h"
#include "gfx_drivers.h"
#include "sync.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int gfx_tween_tag;
static bool s_debug;             /* EMPIRES_TWEEN_DEBUG: log publishes and draws */

#define TWEEN_MAX_OPS   128
#define TWEEN_ARENA     (256 * 1024)
#define TWEEN_W         320
#define TWEEN_H         200
#define TWEEN_HOLD_MS   25.0      /* wait this long for a publish after a newer live present */
#define TWEEN_PREV_MAX_GAP_MS 1000.0
#define TWEEN_RING      4         /* published frames not yet consumed */
#define TWEEN_SLOTS     160       /* distinct (tag, ordinal) objects remembered */
#define TWEEN_SLOT_DATA (12 * 1024)   /* bitmap copy + covered pixels of one op */
#define TWEEN_BEAM_LEN  24
#define TWEEN_BEAM_STEP 8

enum tween_kind { TW_COPY_RECT, TW_VLINE, TW_PIXEL };

struct tween_op {
    int tag;
    enum tween_kind kind;
    dos_int x, y, n;                 /* n: vline count */
    dos_int flip;
    dos_int c94, c96, c98, c9a;      /* clip words in force (copy_rect) */
    uint8_t color;                   /* vline / pixel colour */
    uint32_t bitmap_off;             /* arena: copy of the bitmap (copy_rect) */
    uint32_t bitmap_len;
    uint32_t under_off;              /* arena: pixels covered by the draw */
    uint16_t yv, rows;               /* covered rect: rows */
    uint16_t xb, wb;                 /* covered rect: first byte column, bytes per row */
};

struct tween_frame {
    bool valid;
    bool overflow;                   /* op list or arena ran out: present plainly */
    int nops;
    size_t arena_used;
    double publish_ms, deadline_ms;
    uint32_t generation;
    uint32_t scene_generation;
    uint32_t seq;
    uint8_t vram[TWEEN_W * TWEEN_H];
    struct tween_op ops[TWEEN_MAX_OPS];
    uint8_t arena[TWEEN_ARENA];
};

/* Presenter-side copy of one op with its data and its frame's timing. */
struct tween_hist {
    bool valid;
    struct tween_op op;              /* bitmap_off/under_off relative to data[] */
    double t_pub, t_end;
    uint32_t seq;
    int index;                       /* position inside its publish (draw order) */
    uint8_t data[TWEEN_SLOT_DATA];
};

struct tween_slot {
    bool used;
    int tag, k;
    enum tween_kind kind;
    struct tween_hist latest, previous;
};

static bool s_enabled;
static struct tween_frame *s_staging;            /* game thread */
static struct tween_frame *s_ring[TWEEN_RING];   /* published, under s_lock */
static uint32_t s_ring_head, s_ring_count;
static struct tween_frame *s_base;               /* presenter: newest consumed frame */
static struct tween_slot *s_slots;               /* presenter: TWEEN_SLOTS */
static sync_mutex s_lock;
static bool s_lock_init;
static uint32_t s_seq;
static uint32_t s_scene_generation = 1;
static double s_first_newer_ms = -1.0;
static unsigned s_stat_composed, s_stat_interpolated, s_stat_live, s_stat_published;

void gfx_tween_stats(unsigned *published, unsigned *composed, unsigned *interpolated, unsigned *live)
{
    if (published) *published = s_stat_published;
    if (composed) *composed = s_stat_composed;
    if (interpolated) *interpolated = s_stat_interpolated;
    if (live) *live = s_stat_live;
}

static void frame_reset_ops(struct tween_frame *f)
{
    f->nops = 0;
    f->arena_used = 0;
    f->overflow = false;
}

/* Copy only the used part of a frame. */
static void frame_copy(struct tween_frame *dst, const struct tween_frame *src)
{
    dst->valid = src->valid;
    dst->overflow = src->overflow;
    dst->nops = src->nops;
    dst->arena_used = src->arena_used;
    dst->publish_ms = src->publish_ms;
    dst->deadline_ms = src->deadline_ms;
    dst->generation = src->generation;
    dst->scene_generation = src->scene_generation;
    dst->seq = src->seq;
    memcpy(dst->vram, src->vram, sizeof dst->vram);
    memcpy(dst->ops, src->ops, sizeof(src->ops[0]) * (size_t)src->nops);
    memcpy(dst->arena, src->arena, src->arena_used);
}

static void reset_state_locked(void)
{
    if (s_staging) {
        frame_reset_ops(s_staging);
        s_staging->valid = false;
    }
    for (int i = 0; i < TWEEN_RING; i++)
        if (s_ring[i]) s_ring[i]->valid = false;
    s_ring_head = s_ring_count = 0;
    if (s_base) s_base->valid = false;
    if (s_slots) memset(s_slots, 0, sizeof(s_slots[0]) * TWEEN_SLOTS);
    s_first_newer_ms = -1.0;
    gfx_tween_tag = 0;
}

void gfx_tween_set_enabled(bool enabled)
{
    if (!s_lock_init) {
        sync_mutex_init(&s_lock);
        s_lock_init = true;
        s_debug = getenv("EMPIRES_TWEEN_DEBUG") != NULL;
    }
    if (enabled && !s_staging) {
        bool ok = true;
        s_staging = (struct tween_frame *)calloc(1, sizeof *s_staging);
        s_base = (struct tween_frame *)calloc(1, sizeof *s_base);
        s_slots = (struct tween_slot *)calloc(TWEEN_SLOTS, sizeof *s_slots);
        ok = s_staging && s_base && s_slots;
        for (int i = 0; i < TWEEN_RING; i++) {
            s_ring[i] = (struct tween_frame *)calloc(1, sizeof *s_ring[i]);
            ok = ok && s_ring[i];
        }
        if (!ok) {
            free(s_staging); free(s_base); free(s_slots);
            for (int i = 0; i < TWEEN_RING; i++) { free(s_ring[i]); s_ring[i] = NULL; }
            s_staging = s_base = NULL; s_slots = NULL;
            enabled = false;
        }
    }
    sync_mutex_lock(&s_lock);
    s_enabled = enabled;
    reset_state_locked();
    sync_mutex_unlock(&s_lock);
}

bool gfx_tween_enabled(void)
{
    return s_enabled;
}

void gfx_tween_presenter_resume(void)
{
    if (!s_lock_init)
        return;
    sync_mutex_lock(&s_lock);
    /* While interpolation is off the game thread continues publishing, but
     * the presenter shows live VRAM instead.  Those queued frames and their
     * object history are no longer a valid interpolation origin when the
     * presenter resumes.  Drop only presenter-owned state; leave staging and
     * capture enabled so the game thread is never reset or raced. */
    for (int i = 0; i < TWEEN_RING; i++)
        if (s_ring[i]) s_ring[i]->valid = false;
    s_ring_head = s_ring_count = 0;
    if (s_base)
        s_base->valid = false;
    if (s_slots)
        memset(s_slots, 0, sizeof(s_slots[0]) * TWEEN_SLOTS);
    s_first_newer_ms = -1.0;
    sync_mutex_unlock(&s_lock);
}

void gfx_tween_scene_reset(void)
{
    if (!s_lock_init)
        return;

    sync_mutex_lock(&s_lock);
    /* The staging frame belongs to the game thread, so it is safe to clear
     * it here.  Published frames are handed to the presenter under this same
     * lock.  Presenter-owned base/history is cleared lazily by compose after
     * it observes the generation change; this avoids touching presenter data
     * from the game thread. */
    if (s_staging) {
        frame_reset_ops(s_staging);
        s_staging->valid = false;
    }
    for (int i = 0; i < TWEEN_RING; i++)
        if (s_ring[i]) s_ring[i]->valid = false;
    s_ring_head = s_ring_count = 0;
    if (++s_scene_generation == 0)
        s_scene_generation = 1;
    gfx_tween_tag = 0;
    sync_mutex_unlock(&s_lock);
}

/* ---- capture (game thread) ------------------------------------------- */

static uint8_t *arena_take(struct tween_frame *f, size_t n, uint32_t *off)
{
    if (f->arena_used + n > TWEEN_ARENA) {
        f->overflow = true;
        return NULL;
    }
    *off = (uint32_t)f->arena_used;
    f->arena_used += n;
    return f->arena + *off;
}

static struct tween_op *op_take(struct tween_frame *f)
{
    if (f->nops >= TWEEN_MAX_OPS) {
        f->overflow = true;
        return NULL;
    }
    return &f->ops[f->nops];
}

/* Copy the surface pixels of rows [yv, yv+rows) x bytes [xb, xb+wb). */
static void capture_under(uint8_t *dst, uint16_t yv, uint16_t rows, uint16_t xb, uint16_t wb)
{
    for (uint16_t r = 0; r < rows; r++) {
        uint16_t row = (uint16_t)(yv + r);
        if (row < GFX_ROWS && (unsigned)xb + wb <= 0x140u)
            memcpy(dst + (size_t)r * wb, g3924[row] + xb, wb);
        else
            memset(dst + (size_t)r * wb, 0, wb);
    }
}

void gfx_tween_capture_copy_rect(int tag, dos_int x, dos_int y, const uint8_t *bitmap,
                                 dos_int flip, const struct vga_copy_clip *clip)
{
    struct tween_frame *f = s_staging;
    struct tween_op *op;
    size_t bitmap_len;
    uint8_t *dst;

    if (!s_enabled || !f || f->overflow)
        return;
    op = op_take(f);
    if (!op)
        return;
    bitmap_len = 0x22u + (size_t)bitmap[0x20] * (size_t)bitmap[0x21];
    dst = arena_take(f, bitmap_len, &op->bitmap_off);
    if (!dst)
        return;
    memcpy(dst, bitmap, bitmap_len);
    op->bitmap_len = (uint32_t)bitmap_len;

    op->yv = clip->yv;
    op->rows = clip->dxr;
    op->xb = (uint16_t)(2u * clip->col);
    op->wb = (uint16_t)(2u * clip->cx);
    dst = arena_take(f, (size_t)op->rows * op->wb, &op->under_off);
    if (!dst)
        return;
    capture_under(dst, op->yv, op->rows, op->xb, op->wb);

    op->tag = tag;
    op->kind = TW_COPY_RECT;
    op->x = x; op->y = y; op->n = 0;
    op->flip = flip;
    op->c94 = g94; op->c96 = g96; op->c98 = g98; op->c9a = g9a;
    op->color = 0;
    f->nops++;
}

void gfx_tween_capture_vline(int tag, dos_int x, dos_int y, dos_int n, uint8_t color)
{
    struct tween_frame *f = s_staging;
    struct tween_op *op;
    uint8_t *dst;
    uint16_t count = (uint16_t)n;

    if (!s_enabled || !f || f->overflow)
        return;
    op = op_take(f);
    if (!op)
        return;
    op->yv = (uint16_t)y;
    op->rows = count;
    op->xb = (uint16_t)x;
    op->wb = 1;
    dst = arena_take(f, count, &op->under_off);
    if (!dst)
        return;
    capture_under(dst, op->yv, op->rows, op->xb, 1);

    op->tag = tag;
    op->kind = TW_VLINE;
    op->x = x; op->y = y; op->n = n;
    op->flip = 0;
    op->c94 = op->c96 = op->c98 = op->c9a = 0;
    op->color = color;
    op->bitmap_off = 0; op->bitmap_len = 0;
    f->nops++;
}

void gfx_tween_capture_pixel(int tag, dos_int x, dos_int y, uint8_t color)
{
    struct tween_frame *f = s_staging;
    struct tween_op *op;
    uint8_t *dst;

    if (!s_enabled || !f || f->overflow)
        return;
    op = op_take(f);
    if (!op)
        return;
    op->yv = (uint16_t)y;
    op->rows = 1;
    op->xb = (uint16_t)x;
    op->wb = 1;
    dst = arena_take(f, 1, &op->under_off);
    if (!dst)
        return;
    capture_under(dst, op->yv, 1, op->xb, 1);

    op->tag = tag;
    op->kind = TW_PIXEL;
    op->x = x; op->y = y; op->n = 1;
    op->flip = 0;
    op->c94 = op->c96 = op->c98 = op->c9a = 0;
    op->color = color;
    op->bitmap_off = 0; op->bitmap_len = 0;
    f->nops++;
}

/* ---- publish (game thread) ------------------------------------------- */

void gfx_tween_frame_publish(double now_ms, double deadline_ms, uint32_t vram_generation)
{
    struct tween_frame *f = s_staging;

    if (!s_enabled || !f)
        return;
    memcpy(f->vram, gfx_vram, sizeof f->vram);
    f->publish_ms = now_ms;
    f->deadline_ms = deadline_ms;
    f->generation = vram_generation;
    f->scene_generation = s_scene_generation;
    f->seq = ++s_seq;
    f->valid = true;
    s_stat_published++;
    if (s_debug) {
        fprintf(stderr, "[tween] publish seq=%u window=%.1fms ops=%d%s:", f->seq,
                f->deadline_ms - f->publish_ms, f->nops, f->overflow ? " OVERFLOW" : "");
        for (int i = 0; i < f->nops; i++)
            fprintf(stderr, " %x@(%d,%d%s)", (unsigned)f->ops[i].tag, (int)f->ops[i].x, (int)f->ops[i].y,
                    f->ops[i].kind == TW_VLINE ? ",v" : f->ops[i].kind == TW_PIXEL ? ",p" : "");
        fputc(10, stderr);
    }

    sync_mutex_lock(&s_lock);
    if (s_ring_count == TWEEN_RING) {          /* presenter fell behind: drop the oldest */
        s_ring_head = (s_ring_head + 1) % TWEEN_RING;
        s_ring_count--;
    }
    frame_copy(s_ring[(s_ring_head + s_ring_count) % TWEEN_RING], f);
    s_ring_count++;
    sync_mutex_unlock(&s_lock);

    frame_reset_ops(f);
}

/* ---- presenter: history ------------------------------------------------ */

static struct tween_slot *slot_find(int tag, int k, enum tween_kind kind, bool create)
{
    struct tween_slot *free_slot = NULL, *oldest = NULL;
    for (int i = 0; i < TWEEN_SLOTS; i++) {
        struct tween_slot *s = &s_slots[i];
        if (s->used) {
            if (s->tag == tag && s->k == k && s->kind == kind)
                return s;
            if (!oldest || s->latest.t_pub < oldest->latest.t_pub)
                oldest = s;
        } else if (!free_slot) {
            free_slot = s;
        }
    }
    if (!create)
        return NULL;
    if (!free_slot)
        free_slot = oldest;             /* recycle the least recently drawn object */
    memset(free_slot, 0, sizeof *free_slot);
    free_slot->used = true;
    free_slot->tag = tag;
    free_slot->k = k;
    free_slot->kind = kind;
    return free_slot;
}

static bool hist_store(struct tween_hist *h, const struct tween_frame *f, int index)
{
    const struct tween_op *op = &f->ops[index];
    size_t under_len = (size_t)op->rows * op->wb;
    if (op->bitmap_len + under_len > TWEEN_SLOT_DATA) {
        h->valid = false;
        return false;
    }
    h->op = *op;
    h->op.bitmap_off = 0;
    h->op.under_off = op->bitmap_len;
    memcpy(h->data, f->arena + op->bitmap_off, op->bitmap_len);
    memcpy(h->data + op->bitmap_len, f->arena + op->under_off, under_len);
    h->t_pub = f->publish_ms;
    h->t_end = f->deadline_ms;
    h->seq = f->seq;
    h->index = index;
    h->valid = true;
    return true;
}

/* Fold a consumed frame into the per-object history. */
static void consume_frame(const struct tween_frame *f)
{
    for (int i = 0; i < f->nops; i++) {
        const struct tween_op *op = &f->ops[i];
        struct tween_slot *s;
        int k = 0;
        for (int j = 0; j < i; j++)
            if (f->ops[j].tag == op->tag && f->ops[j].kind == op->kind)
                k++;
        s = slot_find(op->tag, k, op->kind, true);
        s->previous = s->latest;
        hist_store(&s->latest, f, i);
    }
}

/* ---- presenter: composition ------------------------------------------ */

static int lerp_int(dos_int a, dos_int b, double alpha)
{
    double v = (double)a + ((double)b - (double)a) * alpha;
    return (int)(v >= 0 ? v + 0.5 : v - 0.5);
}

static bool is_teleport(dos_int a, dos_int b)
{
    int d = (int)b - (int)a;
    return d > GFX_TWEEN_SNAP_DISTANCE || d < -GFX_TWEEN_SNAP_DISTANCE;
}

static bool is_beam_tag(int tag)
{
    return tag >= GFX_TWEEN_TAG_BEAM(0) && tag < GFX_TWEEN_TAG_BEAM(TWEEN_BEAM_LEN);
}

static double hist_alpha(const struct tween_hist *h, double now_ms)
{
    double a;
    if (h->t_end <= h->t_pub)
        return 1.0;
    a = (now_ms - h->t_pub) / (h->t_end - h->t_pub);
    return a < 0.0 ? 0.0 : a > 1.0 ? 1.0 : a;
}

static void restore_under(uint8_t *out, const struct tween_hist *h)
{
    const struct tween_op *op = &h->op;
    const uint8_t *src = h->data + op->under_off;
    for (uint16_t r = 0; r < op->rows; r++) {
        uint16_t row = (uint16_t)(op->yv + r);
        if (row < TWEEN_H && (unsigned)op->xb + op->wb <= TWEEN_W)
            memcpy(out + (size_t)row * TWEEN_W + op->xb, src + (size_t)r * op->wb, op->wb);
    }
}

static void draw_hist(uint8_t *out, uint8_t *const *rows, const struct tween_hist *h,
                      dos_int x, dos_int y, dos_int n)
{
    const struct tween_op *op = &h->op;
    if (op->kind == TW_COPY_RECT) {
        const uint8_t *bitmap = h->data + op->bitmap_off;
        struct vga_copy_clip c;
        dos_int c94 = op->c94 < 0 ? 0 : op->c94;
        dos_int c96 = op->c96 > TWEEN_H - 1 ? TWEEN_H - 1 : op->c96;
        dos_int c98 = op->c98 < 0 ? 0 : op->c98;
        dos_int c9a = op->c9a > TWEEN_W / 2 - 1 ? TWEEN_W / 2 - 1 : op->c9a;
        bool visible = vga_copy_rect_clip(x, y, bitmap, op->flip, c94, c96, c98, c9a, &c);
        if (visible)
            vga_copy_rect_draw(rows, &c, bitmap + 0x10, op->flip);
        if (s_debug && !visible)
            fprintf(stderr, "[tween] draw %x rec=(%d,%d) at=(%d,%d) flip=%d clip=%d..%d/%d..%d INVISIBLE\n",
                    (unsigned)op->tag, (int)op->x, (int)op->y, (int)x, (int)y, (int)op->flip,
                    (int)op->c94, (int)op->c96, (int)op->c98, (int)op->c9a);
    } else {
        uint16_t count = (uint16_t)n;
        if ((uint16_t)x >= TWEEN_W)
            return;
        for (uint16_t k = 0; k < count; k++) {
            uint16_t row = (uint16_t)((uint16_t)y + k);
            if (row < TWEEN_H)
                out[(size_t)row * TWEEN_W + (uint16_t)x] = op->color;
        }
    }
}

struct active {
    struct tween_slot *slot;
    double alpha;
    bool lerp;              /* previous usable */
    bool hidden;            /* beam head pixel not yet reached */
};

static int cmp_active(const void *a, const void *b)
{
    const struct active *x = (const struct active *)a, *y = (const struct active *)b;
    if (x->slot->latest.seq != y->slot->latest.seq)
        return x->slot->latest.seq < y->slot->latest.seq ? -1 : 1;
    return x->slot->latest.index - y->slot->latest.index;
}

bool gfx_tween_compose(uint8_t *out, double now_ms, uint32_t live_generation)
{
    uint8_t *rows[TWEEN_H];
    struct active act[TWEEN_SLOTS];
    int nact = 0;
    bool any_motion = false;
    const struct tween_frame *base;

    if (!s_enabled || !s_base)
        return false;

    /* Consume every frame published since the last look, in order. */
    sync_mutex_lock(&s_lock);
    if (s_base->valid && s_base->scene_generation != s_scene_generation) {
        s_base->valid = false;
        memset(s_slots, 0, sizeof(s_slots[0]) * TWEEN_SLOTS);
        s_first_newer_ms = -1.0;
    }
    while (s_ring_count > 0) {
        struct tween_frame *f = s_ring[s_ring_head];
        consume_frame(f);
        frame_copy(s_base, f);
        s_ring_head = (s_ring_head + 1) % TWEEN_RING;
        s_ring_count--;
        s_first_newer_ms = -1.0;
    }
    sync_mutex_unlock(&s_lock);

    base = s_base;
    if (!base->valid)
        return false;

    if (live_generation != base->generation) {
        /* The game presented something after this frame.  Either the next
         * frame's flush (its publish follows within microseconds) or a
         * non-frame context: hold the finished frame briefly, then give up
         * and let the live VRAM through. */
        if (s_first_newer_ms < 0)
            s_first_newer_ms = now_ms;
        if (now_ms - s_first_newer_ms > TWEEN_HOLD_MS) {
            s_stat_live++;
            return false;
        }
        now_ms = 1e300;                       /* finished: everything at its final position */
    } else {
        s_first_newer_ms = -1.0;
    }

    memcpy(out, base->vram, (size_t)TWEEN_W * TWEEN_H);
    s_stat_composed++;
    if (base->overflow)
        return true;

    /* Active objects: those of the newest frame, plus older ones whose own
     * window is still running (event-driven animation). */
    for (int i = 0; i < TWEEN_SLOTS; i++) {
        struct tween_slot *s = &s_slots[i];
        struct active *a;
        if (!s->used || !s->latest.valid)
            continue;
        if (s->latest.seq != base->seq && (is_beam_tag(s->tag) || !(s->latest.t_end > base->publish_ms)))
            continue;
        a = &act[nact];
        a->slot = s;
        a->alpha = hist_alpha(&s->latest, now_ms);
        a->hidden = false;
        a->lerp = false;
        if (is_beam_tag(s->tag)) {
            /* head pixels appear progressively: age 7 first, age 0 last */
            int age = s->tag - GFX_TWEEN_TAG_BEAM(0);
            if (age < TWEEN_BEAM_STEP && (double)(TWEEN_BEAM_STEP - 1 - age) >= a->alpha * TWEEN_BEAM_STEP) {
                a->hidden = true;
                any_motion = true;
            }
        } else if (s->previous.valid && s->latest.t_pub - s->previous.t_pub < TWEEN_PREV_MAX_GAP_MS) {
            const struct tween_op *p = &s->previous.op, *c = &s->latest.op;
            if (!is_teleport(p->x, c->x) && !is_teleport(p->y, c->y) &&
                (c->kind != TW_VLINE || !is_teleport(p->n, c->n)) &&
                (p->x != c->x || p->y != c->y || p->n != c->n)) {
                a->lerp = true;
                if (a->alpha < 1.0)
                    any_motion = true;
            }
        }
        nact++;
    }
    /* Beam tail: pixels the newest frame dropped linger, oldest first to go. */
    {
        struct tween_slot *head = slot_find(GFX_TWEEN_TAG_BEAM(0), 0, TW_PIXEL, false);
        if (head && head->latest.valid && head->latest.seq == base->seq && hist_alpha(&head->latest, now_ms) < 1.0)
            any_motion = true;
    }

    if (!any_motion)
        return true;                          /* the frame exactly as presented */
    s_stat_interpolated++;

    for (int i = 0; i < TWEEN_H; i++)
        rows[i] = out + (size_t)i * TWEEN_W;
    qsort(act, (size_t)nact, sizeof act[0], cmp_active);

    for (int i = nact - 1; i >= 0; i--)
        restore_under(out, &act[i].slot->latest);

    /* Beam tail pixels from the previous frame (ages 16..23 there). */
    for (int age = TWEEN_BEAM_LEN - 1; age >= TWEEN_BEAM_LEN - TWEEN_BEAM_STEP; age--) {
        struct tween_slot *s = slot_find(GFX_TWEEN_TAG_BEAM(age), 0, TW_PIXEL, false);
        const struct tween_hist *p;
        double alpha;
        if (!s || !s->latest.valid || s->latest.seq != base->seq || !s->previous.valid)
            continue;
        p = &s->previous;
        if (s->latest.t_pub - p->t_pub >= TWEEN_PREV_MAX_GAP_MS)
            continue;
        alpha = hist_alpha(&s->latest, now_ms);
        /* the tail has advanced alpha*8 points: previous-frame age a (0 =
         * that frame's head) is still there while (23 - a) >= alpha*8 */
        if ((double)(TWEEN_BEAM_LEN - 1 - age) >= alpha * TWEEN_BEAM_STEP)
            draw_hist(out, rows, p, p->op.x, p->op.y, 1);
    }

    for (int i = 0; i < nact; i++) {
        const struct active *a = &act[i];
        const struct tween_hist *h = &a->slot->latest;
        dos_int x = h->op.x, y = h->op.y, n = h->op.n;
        if (a->hidden)
            continue;
        if (a->lerp) {
            const struct tween_op *p = &a->slot->previous.op;
            x = (dos_int)lerp_int(p->x, h->op.x, a->alpha);
            y = (dos_int)lerp_int(p->y, h->op.y, a->alpha);
            if (h->op.kind == TW_VLINE)
                n = (dos_int)lerp_int(p->n, h->op.n, a->alpha);
        }
        draw_hist(out, rows, h, x, y, n);
    }
    return true;
}
