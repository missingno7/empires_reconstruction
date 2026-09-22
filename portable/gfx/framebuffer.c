/* framebuffer.c -- gfx state, framebuffer allocation, palette plumbing.
 *
 * Historical source for this file: src/VIDEO.C (F_01BC..F_0355) for the
 * palette/mode helpers and video_alloc_framebuffer, plus the DGROUP objects
 * portable/gfx owns per tools/portable/state_ownership.json.  See
 * docs/portable/architecture.md "Video model" and portable/include/gfx.h
 * for the contract; this file only defines state and non-primitive helpers.
 * The drawing primitives (asm/RUNTIME_BLOCK.ASM) live in primitives.c.
 */
#include "gfx.h"
#include "game_data.h"   /* g94..g9a, gbc, g9c, gbe, gde, gfe: generated DATA */
#include "sync.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/* portable/resource owns display_mode (DS:BFCD); src/VIDEO.C's `mode` is
 * the same DGROUP object under an unsigned-char view (see VIDEO.C's own
 * comment).  We keep one object and alias the historical second name. */
extern dos_char display_mode;
#define mode display_mode

/* ---- DGROUP objects this module owns (tools/portable/state_ownership.json) */
uint8_t *g3924[GFX_ROWS];
dos_int  result;
uint8_t *rect_queue_write_ptr;
const uint8_t *gc0e0;
dos_uint gc0de;
dos_uint gc0e2;
dos_uint gc0e4;
dos_uint gc0e6;
dos_int  dialog_line_height;

uint8_t gfx_vram[GFX_VRAM_W * GFX_VRAM_H];
uint8_t gfx_dac[256 * 3];
uint32_t gfx_vram_generation;

dos_int cur_idx;             /* DS:3902 */
#define g3902 cur_idx        /* src/VIDEO.C's second name for the same word */
dos_int g3904[16];           /* DS:3904 */


/* Raw allocation backing g3924[]; historical DS:40CA/40CC (g40ca) held the
 * far pointer to this block.  Not part of the public interface: nothing
 * outside this file needs the DOS-style far-pointer/paragraph-normalize
 * dance video_alloc_framebuffer did (src/VIDEO.C F_0281); we allocate one
 * flat contiguous block instead (rows ARE contiguous per the architecture
 * doc, so `pointer += gfx_row_bytes()` from any row is legitimate). */
static uint8_t *g40ca;

/* Row stride of the block g40ca currently points at (src/VIDEO.C
 * video_alloc_framebuffer: w = 0x140 for display_mode 5, 0x50 for
 * display_mode 2, 0xA0 otherwise), set by the last gfx_framebuffer_init(). */
static dos_int s_gfx_row_bytes = GFX_ROW_BYTES;

dos_int gfx_row_bytes(void)
{
    return s_gfx_row_bytes;
}

/* Backing store for the dirty-rect queue that rect_queue_write_ptr walks.
 * The historical queue's capacity is owned by whatever consumes it
 * (unbuilt on this branch); portable/gfx just needs somewhere for the
 * pointer to advance into so gfx_wipe_rect/gfx_blit_bitmap/gfx_copy_rect's
 * `stosw`-pair appends have a valid destination during bring-up and tests.
 * 4 bytes/record, generous headroom for a full-screen worth of dirty rows. */
#define RECT_QUEUE_CAPACITY (4u * 1024u)
static uint8_t s_rect_queue[RECT_QUEUE_CAPACITY];

/* DOS drew directly into visible VRAM. SDL uploads that VRAM from the main
 * thread, so a tight legacy animation loop otherwise overwrites all
 * intermediate frames before the presenter can run. The front end enables
 * this barrier; tests and non-SDL callers leave it disabled. */
static sync_mutex s_present_sync_mutex;
static sync_cond s_present_sync_cond;
static bool s_present_sync_initialized;
static bool s_present_sync_enabled;
static bool s_present_sync_requested;
static uint32_t s_present_sync_presented;

static void present_sync_init(void)
{
    if (s_present_sync_initialized)
        return;
    sync_mutex_init(&s_present_sync_mutex);
    sync_cond_init(&s_present_sync_cond);
    s_present_sync_initialized = true;
}

void gfx_present_sync_set_enabled(bool enabled)
{
    present_sync_init();
    sync_mutex_lock(&s_present_sync_mutex);
    s_present_sync_enabled = enabled;
    s_present_sync_requested = false;
    sync_cond_broadcast(&s_present_sync_cond);
    sync_mutex_unlock(&s_present_sync_mutex);
}

void gfx_present_sync_begin(void)
{
    if (!s_present_sync_initialized)
        return;
    sync_mutex_lock(&s_present_sync_mutex);
    if (s_present_sync_enabled)
        s_present_sync_requested = true;
    sync_mutex_unlock(&s_present_sync_mutex);
}

void gfx_present_sync_wait(void)
{
    uint32_t target;

    if (!s_present_sync_initialized)
        return;
    sync_mutex_lock(&s_present_sync_mutex);
    if (!s_present_sync_enabled || !s_present_sync_requested) {
        sync_mutex_unlock(&s_present_sync_mutex);
        return;
    }
    target = gfx_vram_generation;
    while (s_present_sync_enabled && s_present_sync_presented < target)
        sync_cond_wait(&s_present_sync_cond, &s_present_sync_mutex);
    s_present_sync_requested = false;
    sync_mutex_unlock(&s_present_sync_mutex);
}

bool gfx_present_sync_requested(void)
{
    bool requested;

    if (!s_present_sync_initialized)
        return false;
    sync_mutex_lock(&s_present_sync_mutex);
    requested = s_present_sync_enabled && s_present_sync_requested;
    sync_mutex_unlock(&s_present_sync_mutex);
    return requested;
}

void gfx_present_sync_ack(uint32_t generation)
{
    if (!s_present_sync_initialized)
        return;
    sync_mutex_lock(&s_present_sync_mutex);
    if (generation > s_present_sync_presented)
        s_present_sync_presented = generation;
    sync_cond_broadcast(&s_present_sync_cond);
    sync_mutex_unlock(&s_present_sync_mutex);
}

/* ---- framebuffer allocation (src/VIDEO.C F_0281 video_alloc_framebuffer:
 * w = 0x140 for display_mode 5, 0x50 for display_mode 2, 0xA0 otherwise;
 * 488 rows always).  The historical far-pointer normalize/farmalloc dance
 * is DOS segment plumbing with no portable meaning; we keep only its
 * observable effect (488 row pointers, each gfx_row_bytes() apart, into one
 * zeroed block) and skip video_load_palette here per the task contract
 * ("do not load palette here"). */
void gfx_framebuffer_init(void)
{
    if (g40ca != NULL) {
        gfx_framebuffer_shutdown();
    }

    if (mode == 5)      s_gfx_row_bytes = GFX_ROW_BYTES_VGA;
    else if (mode == 2) s_gfx_row_bytes = 0x50;
    else                s_gfx_row_bytes = GFX_ROW_BYTES;

    g40ca = (uint8_t *)calloc((size_t)s_gfx_row_bytes * GFX_ROWS, 1);
    for (int i = 0; i < GFX_ROWS; i++) {
        g3924[i] = g40ca + (size_t)i * s_gfx_row_bytes;
    }

    rect_queue_write_ptr = s_rect_queue;
    memset(s_rect_queue, 0, sizeof s_rect_queue);

    memset(gfx_vram, 0, sizeof gfx_vram);
    memset(gfx_dac, 0, sizeof gfx_dac);
    gfx_vram_generation = 0;
}

void gfx_framebuffer_shutdown(void)
{
    free(g40ca);
    g40ca = NULL;
    for (int i = 0; i < GFX_ROWS; i++) {
        g3924[i] = NULL;
    }
    rect_queue_write_ptr = NULL;
}

uint8_t *gfx_framebuffer(void)
{
    return g3924[0];
}

/* ---- src/VIDEO.C F_01CE: gfx_color_select. */
void gfx_color_select(dos_int i)
{
    cur_idx = i;
    if (mode == 5)      result = g3904[i];
    else if (mode == 2) result = gbe[i];
    else                result = gfe[i];
}

/* ---- src/VIDEO.C F_020F: cur_color_index_get. */
dos_int cur_color_index_get(void)
{
    return g3902;
}

/* ---- src/VIDEO.C F_0215: color_table_entry_set. */
void color_table_entry_set(dos_int index, dos_int value1, dos_int value2)
{
    g3904[index] = value1;
    g00be[index] = value2;
}

/* ---- src/VIDEO.C F_0232: color_lookup_tables_init (movmem g9c/gde, 0x20
 * bytes each, into g3904/gbe).  Split into a testable `_from` helper per
 * the task contract; color_lookup_tables_init() itself wires the two
 * historical DS:9C/DS:DE source tables (see gfx_data_stub.c). */
void color_lookup_tables_init_from(const uint8_t *g9c_src, const uint8_t *gde_src)
{
    memcpy((void *)g3904, g9c_src, 0x20);
    memcpy((void *)gbe, gde_src, 0x20);
}

void color_lookup_tables_init(void)
{
    color_lookup_tables_init_from((const uint8_t *)g9c, (const uint8_t *)gde);
}

/* ---- src/VIDEO.C F_01BC: video_load_palette.  The historical routine
 * pushed a far pointer through INT 10h AX=1012h to load 256 6-bit-DAC RGB
 * triples (768 bytes); the port has no BIOS, so this is the copy the BIOS
 * call would have performed. */
void video_load_palette(const uint8_t *dac6_rgb256)
{
    memcpy(gfx_dac, dac6_rgb256, sizeof gfx_dac);
}

/* ---- src/VIDEO.C F_0355: rect_border_draw (4 edges via gfx_bar/gfx_vline,
 * literal). */
void rect_border_draw(dos_int x, dos_int y, dos_int w, dos_int h)
{
    gfx_bar(x, y, w);
    gfx_vline(x, y, h);
    gfx_bar(x, y + h - 1, w);
    gfx_vline(x + w - 1, y, h);
}
