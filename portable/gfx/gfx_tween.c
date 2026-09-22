/* gfx_tween.c -- frame interpolation for the VGA presenter; see gfx_tween.h.
 *
 * Threading: capture and publish run on the game thread; compose runs on
 * the presenter thread.  The only shared object is the published frame,
 * handed over under a mutex by value (a few hundred KB, ten times a
 * second); the presenter keeps private copies of the current and previous
 * frames, so composition never holds the lock.
 */
#include "gfx_tween.h"
#include "gfx.h"
#include "gfx_drivers.h"
#include "sync.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int gfx_tween_tag;
static bool s_debug;             /* EMPIRES_TWEEN_DEBUG: log publishes */

#define TWEEN_MAX_OPS   96
#define TWEEN_ARENA     (256 * 1024)
#define TWEEN_W         320
#define TWEEN_H         200
#define TWEEN_HOLD_MS   40.0      /* wait this long for a publish after a newer live present */
#define TWEEN_PREV_MAX_GAP_MS 400.0

enum tween_kind { TW_COPY_RECT, TW_VLINE };

struct tween_op {
    int tag;
    enum tween_kind kind;
    dos_int x, y, n;                 /* n: vline count */
    dos_int flip;
    dos_int c94, c96, c98, c9a;      /* clip words in force (copy_rect) */
    uint8_t color;                   /* vline colour */
    uint32_t bitmap_off;             /* arena: copy of the bitmap (copy_rect) */
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
    uint32_t seq;
    uint8_t vram[TWEEN_W * TWEEN_H];
    struct tween_op ops[TWEEN_MAX_OPS];
    uint8_t arena[TWEEN_ARENA];
};

static bool s_enabled;
static struct tween_frame *s_staging;    /* game thread */
static struct tween_frame *s_published;  /* handed over under s_lock, with the one before it */
static struct tween_frame *s_published_prev;
static struct tween_frame *s_cur, *s_prev; /* presenter thread */
static sync_mutex s_lock;
static bool s_lock_init;
static uint32_t s_seq;
static double s_first_newer_ms = -1.0;
static unsigned s_stat_composed, s_stat_interpolated, s_stat_live, s_stat_published;

void gfx_tween_stats(unsigned *published, unsigned *composed, unsigned *interpolated, unsigned *live)
{
    if (published) *published = s_stat_published;
    if (composed) *composed = s_stat_composed;
    if (interpolated) *interpolated = s_stat_interpolated;
    if (live) *live = s_stat_live;
}

static struct tween_frame *frame_alloc(void)
{
    struct tween_frame *f = (struct tween_frame *)calloc(1, sizeof *f);
    return f;
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
    dst->seq = src->seq;
    memcpy(dst->vram, src->vram, sizeof dst->vram);
    memcpy(dst->ops, src->ops, sizeof(src->ops[0]) * (size_t)src->nops);
    memcpy(dst->arena, src->arena, src->arena_used);
}

void gfx_tween_set_enabled(bool enabled)
{
    if (!s_lock_init) {
        sync_mutex_init(&s_lock);
        s_lock_init = true;
        s_debug = getenv("EMPIRES_TWEEN_DEBUG") != NULL;
    }
    if (enabled && !s_staging) {
        s_staging = frame_alloc();
        s_published = frame_alloc();
        s_published_prev = frame_alloc();
        s_cur = frame_alloc();
        s_prev = frame_alloc();
        if (!s_staging || !s_published || !s_published_prev || !s_cur || !s_prev) {
            free(s_staging); free(s_published); free(s_published_prev); free(s_cur); free(s_prev);
            s_staging = s_published = s_published_prev = s_cur = s_prev = NULL;
            enabled = false;
        }
    }
    sync_mutex_lock(&s_lock);
    s_enabled = enabled;
    if (s_staging) {
        frame_reset_ops(s_staging);
        s_staging->valid = false;
        s_published->valid = false;
        s_published_prev->valid = false;
        s_cur->valid = false;
        s_prev->valid = false;
    }
    s_first_newer_ms = -1.0;
    gfx_tween_tag = 0;
    sync_mutex_unlock(&s_lock);
}

bool gfx_tween_enabled(void)
{
    return s_enabled;
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
    op->bitmap_off = 0;
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
    f->seq = ++s_seq;
    f->valid = true;
    s_stat_published++;
    if (s_debug) {
        fprintf(stderr, "[tween] publish seq=%u ops=%d%s:", f->seq, f->nops, f->overflow ? " OVERFLOW" : "");
        for (int i = 0; i < f->nops; i++)
            fprintf(stderr, " %x@(%d,%d%s)", (unsigned)f->ops[i].tag, (int)f->ops[i].x, (int)f->ops[i].y,
                    f->ops[i].kind == TW_VLINE ? ",v" : "");
        fputc(10, stderr);
    }

    sync_mutex_lock(&s_lock);
    {
        /* Keep the previous frame with the new one so the presenter can
         * interpolate even when it did not get to look between two
         * publishes. */
        struct tween_frame *t = s_published_prev;
        s_published_prev = s_published;
        s_published = t;
    }
    frame_copy(s_published, f);
    sync_mutex_unlock(&s_lock);

    frame_reset_ops(f);
}

/* ---- compose (presenter thread) -------------------------------------- */

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

/* The k-th op of the same tag and kind in `f`, or NULL. */
static const struct tween_op *find_match(const struct tween_frame *f, int tag, enum tween_kind kind, int k)
{
    for (int i = 0; i < f->nops; i++) {
        const struct tween_op *op = &f->ops[i];
        if (op->tag == tag && op->kind == kind) {
            if (k == 0)
                return op;
            k--;
        }
    }
    return NULL;
}

static void restore_under(uint8_t *out, const struct tween_frame *f, const struct tween_op *op)
{
    const uint8_t *src = f->arena + op->under_off;
    for (uint16_t r = 0; r < op->rows; r++) {
        uint16_t row = (uint16_t)(op->yv + r);
        if (row < TWEEN_H && (unsigned)op->xb + op->wb <= TWEEN_W)
            memcpy(out + (size_t)row * TWEEN_W + op->xb, src + (size_t)r * op->wb, op->wb);
    }
}

static void draw_op(uint8_t *out, uint8_t *const *rows, const struct tween_frame *f,
                    const struct tween_op *op, dos_int x, dos_int y, dos_int n)
{
    if (op->kind == TW_COPY_RECT) {
        const uint8_t *bitmap = f->arena + op->bitmap_off;
        struct vga_copy_clip c;
        dos_int c94 = op->c94 < 0 ? 0 : op->c94;
        dos_int c96 = op->c96 > TWEEN_H - 1 ? TWEEN_H - 1 : op->c96;
        dos_int c98 = op->c98 < 0 ? 0 : op->c98;
        dos_int c9a = op->c9a > TWEEN_W / 2 - 1 ? TWEEN_W / 2 - 1 : op->c9a;
        if (vga_copy_rect_clip(x, y, bitmap, op->flip, c94, c96, c98, c9a, &c))
            vga_copy_rect_draw(rows, &c, bitmap + 0x10, op->flip);
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

bool gfx_tween_compose(uint8_t *out, double now_ms, uint32_t live_generation)
{
    uint8_t *rows[TWEEN_H];
    const struct tween_frame *cur, *prev;
    bool prev_ok;
    double alpha;

    if (!s_enabled || !s_cur)
        return false;

    /* Take over a newer published frame. */
    sync_mutex_lock(&s_lock);
    if (s_published->valid && s_published->seq != s_cur->seq) {
        frame_copy(s_cur, s_published);
        if (s_published_prev->valid)
            frame_copy(s_prev, s_published_prev);
        else
            s_prev->valid = false;
        s_first_newer_ms = -1.0;
    }
    sync_mutex_unlock(&s_lock);

    cur = s_cur;
    prev = s_prev;
    if (!cur->valid)
        return false;

    if (live_generation != cur->generation) {
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
        alpha = 1.0;
    } else {
        s_first_newer_ms = -1.0;
        if (cur->deadline_ms <= cur->publish_ms)
            alpha = 1.0;
        else {
            alpha = (now_ms - cur->publish_ms) / (cur->deadline_ms - cur->publish_ms);
            if (alpha < 0.0) alpha = 0.0;
            if (alpha > 1.0) alpha = 1.0;
        }
    }

    memcpy(out, cur->vram, (size_t)TWEEN_W * TWEEN_H);
    s_stat_composed++;
    if (cur->nops == 0 || cur->overflow)
        return true;

    prev_ok = prev->valid && !prev->overflow && prev->seq + 1 == cur->seq &&
              cur->publish_ms - prev->publish_ms < TWEEN_PREV_MAX_GAP_MS;
    if (!prev_ok || alpha >= 1.0)
        return true;                      /* frame n as presented */

    s_stat_interpolated++;
    for (int i = 0; i < TWEEN_H; i++)
        rows[i] = out + (size_t)i * TWEEN_W;

    /* Erase every captured draw, newest first, then redraw in order at the
     * interpolated positions. */
    for (int i = cur->nops - 1; i >= 0; i--)
        restore_under(out, cur, &cur->ops[i]);

    for (int i = 0; i < cur->nops; i++) {
        const struct tween_op *op = &cur->ops[i];
        const struct tween_op *po;
        dos_int x = op->x, y = op->y, n = op->n;
        int k = 0;
        for (int j = 0; j < i; j++)
            if (cur->ops[j].tag == op->tag && cur->ops[j].kind == op->kind)
                k++;
        po = find_match(prev, op->tag, op->kind, k);
        if (po && !is_teleport(po->x, op->x) && !is_teleport(po->y, op->y) &&
            (op->kind != TW_VLINE || !is_teleport(po->n, op->n))) {
            x = (dos_int)lerp_int(po->x, op->x, alpha);
            y = (dos_int)lerp_int(po->y, op->y, alpha);
            if (op->kind == TW_VLINE)
                n = (dos_int)lerp_int(po->n, op->n, alpha);
        }
        draw_op(out, rows, cur, op, x, y, n);
    }
    return true;
}
