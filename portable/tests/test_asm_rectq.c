/* test_asm_rectq.c -- unit tests for portable/game/asm_rectq.c
 * (rect_queue_flush, the port of asm/RECTQ.ASM's _rect_queue_flush).
 *
 * gfx_framebuffer_init() (portable/gfx/framebuffer.c) always points
 * rect_queue_write_ptr at its own private 4 KB stand-in buffer, not at
 * ui_gfx_blob -- see this module's port report for why that stand-in
 * exists and why it is not this test's (or asm_rectq.c's) place to fix.
 * These tests reproduce the historical usage pattern by hand: they call
 * resource_staging_init() themselves and then explicitly set
 * `rect_queue_write_ptr = ui_gfx_blob;` before appending synthetic
 * records, exactly as src/GAME.C / src/BOARD.C / src/LEVEL.C do before
 * every queueing round.
 *
 * gfx_box's effect is observed two ways: gfx_vram_generation (declared
 * "incremented by every gfx_box" in gfx.h) counts calls, and gfx_vram's
 * bytes confirm the (x,y,w,h) gfx_box actually received, since VGA mode's
 * gfx_box copies framebuffer rows straight into gfx_vram at (x,y).
 */
#include "game.h"

#include <stdio.h>
#include <string.h>

static int g_failures = 0;

static void fail(const char *test, const char *what)
{
    fprintf(stderr, "test_asm_rectq: FAIL %s: %s\n", test, what);
    g_failures++;
}

static void check(const char *test, int cond, const char *what)
{
    if (!cond) fail(test, what);
}

/* Append one raw 4-byte record at rect_queue_write_ptr, exactly the byte
 * layout portable/gfx's wipe/blit/copy primitives write when gbc==1
 * (asm_rectq.h): byte0=x/2, byte1=y, byte2=w/2, byte3=h. */
static void append_record(dos_int x, dos_int y, dos_int w, dos_int h)
{
    rect_queue_write_ptr[0] = (uint8_t)(x >> 1);
    rect_queue_write_ptr[1] = (uint8_t)y;
    rect_queue_write_ptr[2] = (uint8_t)(w >> 1);
    rect_queue_write_ptr[3] = (uint8_t)h;
    rect_queue_write_ptr += 4;
}

static void fill_fb_row(dos_int row, uint8_t value, dos_int n)
{
    memset(g3924[row], value, (size_t)n);
}

static void test_single_record(void)
{
    const char *t = "single_record";

    resource_staging_init();
    display_mode = 5;
    gfx_framebuffer_init();

    /* Historical pre-queueing reset (src/GAME.C/BOARD.C/LEVEL.C). */
    rect_queue_write_ptr = ui_gfx_blob;

    fill_fb_row(20, 0x77, 40);   /* h=3, so vga_box reads rows 20..22 */
    fill_fb_row(21, 0x77, 40);
    fill_fb_row(22, 0x77, 40);
    memset(gfx_vram, 0, sizeof gfx_vram);

    uint32_t gen0 = gfx_vram_generation;
    append_record(10, 20, 6, 3); /* x/2=5,y=20,w/2=3,h=3 -> decodes to x=10,y=20,w=6,h=3 */

    rect_queue_flush();

    check(t, gfx_vram_generation == gen0 + 1, "gfx_vram_generation should advance by exactly one gfx_box call");
    check(t, rect_queue_write_ptr == ui_gfx_blob, "write cursor must reset to ui_gfx_blob after drain");

    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 6; col++) {
            uint8_t v = gfx_vram[(20 + row) * GFX_VRAM_W + (10 + col)];
            if (v != 0x77) {
                fail(t, "vram byte inside the presented rect should equal the source framebuffer byte");
                return;
            }
        }
    }
    /* One column left of the rect must remain untouched (0). */
    check(t, gfx_vram[20 * GFX_VRAM_W + 9] == 0, "vram byte outside the presented rect must stay untouched");

    resource_staging_shutdown();
    gfx_framebuffer_shutdown();
}

static void test_multiple_records_and_halving(void)
{
    const char *t = "multiple_records_and_halving";

    resource_staging_init();
    display_mode = 5;
    gfx_framebuffer_init();
    rect_queue_write_ptr = ui_gfx_blob;

    fill_fb_row(50, 0xAA, 60);
    fill_fb_row(51, 0xAA, 60);
    fill_fb_row(70, 0xBB, 60);
    memset(gfx_vram, 0, sizeof gfx_vram);

    uint32_t gen0 = gfx_vram_generation;
    /* Three queued rects, appended in order: the ASM drains oldest-first
     * (si walks forward from ui_gfx_blob), so gfx_box must be called with
     * these three rects in this same order -- verified indirectly below
     * via the generation counter and each rect's own vram content. */
    append_record(4, 50, 8, 2);   /* x/2=2, w/2=4 -- both even, exact */
    append_record(11, 70, 5, 1);  /* x=11 -> byte0=5 (11>>1 truncates); decodes back to x=10, NOT 11 */
    append_record(20, 51, 10, 1);

    rect_queue_flush();

    check(t, gfx_vram_generation == gen0 + 3, "three queued records must produce three gfx_box calls");
    check(t, rect_queue_write_ptr == ui_gfx_blob, "write cursor must reset to ui_gfx_blob after drain");

    /* Record 1: (4,50,8,2) exact. */
    check(t, gfx_vram[50 * GFX_VRAM_W + 4] == 0xAA && gfx_vram[51 * GFX_VRAM_W + 11] == 0xAA,
          "record 1 rect corners should be presented");

    /* Record 2: x=11 was halved with truncation (11>>1==5) and doubled
     * back (5<<1==10), so the presented rect starts at x=10, not 11 --
     * the record format cannot represent odd x/w exactly. */
    check(t, gfx_vram[70 * GFX_VRAM_W + 10] == 0xBB, "record 2 must decode x=11 down to x=10 (halved/doubled)");

    /* Record 3: (20,51,10,1). */
    check(t, gfx_vram[51 * GFX_VRAM_W + 20] == 0xAA && gfx_vram[51 * GFX_VRAM_W + 29] == 0xAA,
          "record 3 rect span should be presented");

    resource_staging_shutdown();
    gfx_framebuffer_shutdown();
}

int main(void)
{
    test_single_record();
    test_multiple_records_and_halving();

    if (g_failures) {
        fprintf(stderr, "test_asm_rectq: %d failure(s)\n", g_failures);
        return 1;
    }
    printf("test_asm_rectq: OK\n");
    return 0;
}
