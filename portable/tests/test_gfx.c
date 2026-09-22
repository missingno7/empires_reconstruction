/* test_gfx.c -- self-checking unit tests for portable/gfx (Milestone C).
 *
 * Two layers of coverage:
 *
 *  1. Hand-authored, first-principles checks: known-input/known-output
 *     checks for the nibble-precision primitives, and cross-checks between
 *     a primitive under test and an independent "naive" per-pixel
 *     reference built from gfx_set_pixel/gfx_get_pixel (themselves a
 *     from-scratch transcription of a *different* ASM routine,
 *     runtime_f03d2/f03d5, so agreement is a meaningful check on
 *     indexing/parity, not a tautology).
 *
 *  2. A loader/replayer for fixtures/gfx_cases.json (produced by a
 *     separate agent's DOS-oracle harness; see run_gfx_cases_if_present()
 *     below), which runs automatically once that file exists and prints a
 *     note and skips cleanly if it doesn't.
 */
#include "gfx.h"
#include "sha256.h"

#include <stdio.h>
#include <string.h>

#ifdef _MSC_VER
#include <crtdbg.h>
#endif
#include <stdlib.h>

/* portable/resource owns display_mode (DS:BFCD); selects between the
 * packed-4bpp driver (gfx_planar.c, modes 1/3/4) and the 8bpp VGA driver
 * (gfx_vga.c, mode 5) inside every gfx_<name>() dispatcher. */
extern dos_char display_mode;

static int g_failures = 0;

static void fail(const char *test, const char *what)
{
    fprintf(stderr, "test_gfx: FAIL %s: %s\n", test, what);
    g_failures++;
}

static void check(const char *test, int cond, const char *what)
{
    if (!cond) fail(test, what);
}

/* ------------------------------------------------------------------ */
/* Framebuffer helpers                                                 */
/* ------------------------------------------------------------------ */

/* Both helpers use gfx_row_bytes() (not the GFX_ROW_BYTES compile-time
 * constant) so they size correctly whichever driver the last
 * gfx_framebuffer_init() allocated for: 0xA0 bytes/row for the planar
 * driver (display_mode 4), 0x140 for the VGA driver (display_mode 5). */
static void fb_zero(void)
{
    memset(gfx_framebuffer(), 0, (size_t)gfx_row_bytes() * GFX_ROWS);
}

/* Deterministic LCG seed fill, same recurrence the fixture format uses:
 * x = x*1103515245+12345 (mod 2^32), byte = (x>>16)&0xFF, row-major over
 * the whole framebuffer (488*gfx_row_bytes() bytes). */
static void fb_seed(uint32_t seed)
{
    uint8_t *p = gfx_framebuffer();
    size_t n = (size_t)gfx_row_bytes() * GFX_ROWS;
    uint32_t x = seed;
    for (size_t i = 0; i < n; i++) {
        x = x * 1103515245u + 12345u;
        p[i] = (uint8_t)(x >> 16);
    }
}

/* Naive per-pixel reference painter: n consecutive pixels starting at
 * (x,y), using gfx_set_pixel (a separately transcribed primitive). */
static void ref_paint_span(dos_int x, dos_int y, dos_int n, dos_int color)
{
    dos_int saved = result;
    result = color;
    for (dos_int i = 0; i < n; i++) gfx_set_pixel((dos_int)(x + i), y);
    result = saved;
}

static void ref_paint_vspan(dos_int x, dos_int y, dos_int n, dos_int color)
{
    dos_int saved = result;
    result = color;
    for (dos_int i = 0; i < n; i++) gfx_set_pixel(x, (dos_int)(y + i));
    result = saved;
}

/* Compares an (w,h) rect at (x1,y1) against one at (x2,y2) pixel-by-pixel
 * via gfx_get_pixel.  Returns 1 if identical. */
static int rects_equal(dos_int x1, dos_int y1, dos_int x2, dos_int y2, dos_int w, dos_int h)
{
    for (dos_int r = 0; r < h; r++) {
        for (dos_int c = 0; c < w; c++) {
            if (gfx_get_pixel((dos_int)(x1 + c), (dos_int)(y1 + r)) !=
                gfx_get_pixel((dos_int)(x2 + c), (dos_int)(y2 + r))) {
                return 0;
            }
        }
    }
    return 1;
}

/* Mirror-compares: dest(x2+w-1-c, y2+r) == src(x1+c, y1+r). */
static int rects_equal_mirrored_h(dos_int x1, dos_int y1, dos_int x2, dos_int y2, dos_int w, dos_int h)
{
    for (dos_int r = 0; r < h; r++) {
        for (dos_int c = 0; c < w; c++) {
            dos_int a = gfx_get_pixel((dos_int)(x1 + c), (dos_int)(y1 + r));
            dos_int b = gfx_get_pixel((dos_int)(x2 + w - 1 - c), (dos_int)(y2 + r));
            if (a != b) return 0;
        }
    }
    return 1;
}

static int rects_equal_mirrored_v(dos_int x1, dos_int y1, dos_int x2, dos_int y2, dos_int w, dos_int h)
{
    for (dos_int r = 0; r < h; r++) {
        for (dos_int c = 0; c < w; c++) {
            dos_int a = gfx_get_pixel((dos_int)(x1 + c), (dos_int)(y1 + r));
            dos_int b = gfx_get_pixel((dos_int)(x2 + c), (dos_int)(y2 + h - 1 - r));
            if (a != b) return 0;
        }
    }
    return 1;
}

/* ------------------------------------------------------------------ */
/* gfx_bar: nibble-precision at even/odd x, widths 1..5.                */
/* ------------------------------------------------------------------ */

static void test_bar(void)
{
    const char *t = "bar";
    for (dos_int x = 0; x < 6; x++) {
        for (dos_int n = 1; n <= 5; n++) {
            fb_zero();
            result = 0x33;
            gfx_bar(x, 0, n);
            ref_paint_span(x, 1, n, 0x33);
            if (!rects_equal(x, 0, x, 1, n, 1)) {
                fprintf(stderr, "test_gfx: %s: mismatch at x=%d n=%d\n", t, (int)x, (int)n);
                g_failures++;
            }
            /* no bleed outside [x, x+n) */
            if (x > 0 && gfx_get_pixel((dos_int)(x - 1), 0) != 0) fail(t, "bled left of span");
            if (gfx_get_pixel((dos_int)(x + n), 0) != 0) fail(t, "bled right of span");
        }
    }
}

/* ------------------------------------------------------------------ */
/* gfx_vline                                                            */
/* ------------------------------------------------------------------ */

static void test_vline(void)
{
    const char *t = "vline";
    for (dos_int x = 0; x < 4; x++) {
        for (dos_int n = 1; n <= 5; n++) {
            fb_zero();
            result = 0x33;
            gfx_vline(x, 0, n);
            ref_paint_vspan((dos_int)(x + 20), 0, n, 0x33);
            if (!rects_equal(x, 0, (dos_int)(x + 20), 0, 1, n)) {
                fprintf(stderr, "test_gfx: %s: mismatch at x=%d n=%d\n", t, (int)x, (int)n);
                g_failures++;
            }
            if (gfx_get_pixel(x, n) != 0) fail(t, "bled below span");
        }
    }
}

/* ------------------------------------------------------------------ */
/* gfx_clear_rect vs a naive per-pixel fill, many random rects.        */
/* ------------------------------------------------------------------ */

static uint32_t g_rng = 12345u;
static uint32_t next_rand(void)
{
    g_rng = g_rng * 1103515245u + 12345u;
    return (g_rng >> 16) & 0x7FFFu;
}

static void test_clear_rect(void)
{
    const char *t = "clear_rect";
    for (int iter = 0; iter < 60; iter++) {
        dos_int x = (dos_int)(next_rand() % 20);
        dos_int y = (dos_int)(next_rand() % 30);
        dos_int w = (dos_int)(1 + next_rand() % 12);
        dos_int h = (dos_int)(1 + next_rand() % 8);
        dos_int x2 = (dos_int)(x + 60); /* naive copy lives in a disjoint column band */

        fb_zero();
        result = 0x77;
        gfx_clear_rect(x, y, w, h);
        ref_paint_span(0, 0, 0, 0); /* no-op, keeps ref_paint_span referenced */
        for (dos_int r = 0; r < h; r++) ref_paint_span(x2, (dos_int)(y + r), w, 0x77);

        if (!rects_equal(x, y, x2, y, w, h)) {
            fprintf(stderr, "test_gfx: %s: mismatch iter=%d x=%d y=%d w=%d h=%d\n",
                    t, iter, (int)x, (int)y, (int)w, (int)h);
            g_failures++;
        }
    }
}

/* ------------------------------------------------------------------ */
/* gfx_fill_rect (XOR): toggling twice restores the original, and a    */
/* single toggle matches a naive XOR-via-get/set_pixel loop.           */
/* ------------------------------------------------------------------ */

static void test_fill_rect(void)
{
    const char *t = "fill_rect";
    for (int iter = 0; iter < 30; iter++) {
        dos_int x = (dos_int)(next_rand() % 20);
        dos_int y = (dos_int)(next_rand() % 30);
        dos_int w = (dos_int)(1 + next_rand() % 12);
        dos_int h = (dos_int)(1 + next_rand() % 8);

        fb_seed(0xC0FFEEu + (uint32_t)iter);
        uint8_t snapshot[16 * 16];
        for (dos_int r = 0; r < h; r++)
            for (dos_int c = 0; c < w; c++)
                snapshot[r * 16 + c] = (uint8_t)gfx_get_pixel((dos_int)(x + c), (dos_int)(y + r));

        gfx_fill_rect(x, y, w, h);
        for (dos_int r = 0; r < h; r++) {
            for (dos_int c = 0; c < w; c++) {
                dos_int expect = (dos_int)((snapshot[r * 16 + c] ^ 0x0F) & 0x0F);
                dos_int got = gfx_get_pixel((dos_int)(x + c), (dos_int)(y + r));
                if (got != expect) {
                    fprintf(stderr, "test_gfx: %s: xor mismatch iter=%d (%d,%d)\n", t, iter, (int)c, (int)r);
                    g_failures++;
                }
            }
        }
        gfx_fill_rect(x, y, w, h); /* toggle back */
        for (dos_int r = 0; r < h; r++) {
            for (dos_int c = 0; c < w; c++) {
                dos_int got = gfx_get_pixel((dos_int)(x + c), (dos_int)(y + r));
                if (got != snapshot[r * 16 + c]) {
                    fprintf(stderr, "test_gfx: %s: double-toggle didn't restore iter=%d\n", t, iter);
                    g_failures++;
                }
            }
        }
    }
}

/* ------------------------------------------------------------------ */
/* gfx_save_rect / gfx_restore_rect round trip.                        */
/* ------------------------------------------------------------------ */

static void test_save_restore(void)
{
    const char *t = "save_restore";
    dos_int x = 3, y = 5, w = 9, h = 6;
    fb_seed(42);
    uint8_t buf[4 + 16 * 8];
    gfx_save_rect(x, y, w, h, buf);

    /* corrupt the region, then restore and compare via pixel readback
     * against a snapshot taken before corruption. */
    dos_int shadow_x = 80;
    for (dos_int r = 0; r < h; r++)
        for (dos_int c = 0; c < w; c++) {
            dos_int v = gfx_get_pixel((dos_int)(x + c), (dos_int)(y + r));
            dos_int saved = result;
            result = v;
            gfx_set_pixel((dos_int)(shadow_x + c), (dos_int)(y + r));
            result = saved;
        }

    result = 0x00;
    gfx_clear_rect(x, y, w, h);
    check(t, rects_equal(x, y, shadow_x, y, w, h) == 0 || w == 0, "corruption step didn't change anything (test bug)");

    gfx_restore_rect(x, y, buf);
    if (!rects_equal(x, y, shadow_x, y, w, h)) {
        fail(t, "restored region does not match pre-corruption snapshot");
    }
}

/* ------------------------------------------------------------------ */
/* gfx_wipe_rect: framebuffer-to-framebuffer copy.                     */
/* ------------------------------------------------------------------ */

static void test_wipe_rect(void)
{
    const char *t = "wipe_rect";
    dos_int sx = 2, sy = 4, w = 10, h = 7, dx = 40, dy = 20;
    fb_seed(7);
    gbc = 0;
    gfx_wipe_rect(sx, sy, w, h, dx, dy);
    if (!rects_equal(sx, sy, dx, dy, w, h)) {
        fail(t, "destination does not match source after wipe");
    }
}

static void test_wipe_rect_dirty_queue(void)
{
    const char *t = "wipe_rect_dirty_queue";
    fb_seed(1);
    gbc = 1;
    uint8_t queue[64];
    /* Save/restore the real queue pointer: gfx_framebuffer_init() pointed
     * it at its own private static buffer, and later code (including the
     * oracle-fixture replay below, for gbc=1 cases) expects that pointer
     * to still be valid -- pointing it at this function's local `queue`
     * and never restoring it would leave a dangling pointer into a freed
     * stack frame once this function returns. */
    uint8_t *saved_queue_ptr = rect_queue_write_ptr;
    memset(queue, 0xAA, sizeof queue);
    rect_queue_write_ptr = queue;
    gfx_wipe_rect(0, 0, 8, 3, 10, 5); /* dy=5 < 0xC8, should append */
    dos_uint w1 = dos_rd16(queue);
    dos_uint w2 = dos_rd16(queue + 2);
    check(t, (w1 >> 8) == 5, "record y byte wrong");
    check(t, (w1 & 0xFF) == (10 >> 1), "record x/2 byte wrong");
    check(t, (w2 >> 8) == 3, "record h byte wrong");
    check(t, (w2 & 0xFF) == (8 >> 1), "record bytes byte wrong");
    check(t, rect_queue_write_ptr == queue + 4, "write pointer did not advance by 4");
    rect_queue_write_ptr = saved_queue_ptr;
    gbc = 0;
}

/* ------------------------------------------------------------------ */
/* Flip copies: mirror comparisons.                                    */
/* ------------------------------------------------------------------ */

static void test_flip_h(void)
{
    fb_seed(99);
    dos_int sx = 4, sy = 2, w = 12, h = 5, dx = 60, dy = 30;
    gfx_copy_rect_flip_h(sx, sy, w, h, dx, dy);
    if (!rects_equal_mirrored_h(sx, sy, dx, dy, w, h)) {
        fail("flip_h", "destination is not a horizontal mirror of the source");
    }
}

static void test_flip_v(void)
{
    fb_seed(123);
    dos_int sx = 4, sy = 2, w = 8, h = 6, dx = 60, dy = 40;
    gfx_copy_rect_flip_v(sx, sy, w, h, dx, dy);
    if (!rects_equal_mirrored_v(sx, sy, dx, dy, w, h)) {
        fail("flip_v", "destination is not a vertical mirror of the source");
    }
}

static void test_flip_hv(void)
{
    fb_seed(55);
    dos_int sx = 4, sy = 2, w = 8, h = 6, dx = 60, dy = 60;
    gfx_copy_rect_flip_hv(sx, sy, w, h, dx, dy);
    for (dos_int r = 0; r < h; r++) {
        for (dos_int c = 0; c < w; c++) {
            dos_int a = gfx_get_pixel((dos_int)(sx + c), (dos_int)(sy + r));
            dos_int b = gfx_get_pixel((dos_int)(dx + w - 1 - c), (dos_int)(dy + h - 1 - r));
            if (a != b) {
                fail("flip_hv", "destination is not a 180-degree mirror of the source");
                return;
            }
        }
    }
}

/* ------------------------------------------------------------------ */
/* gfx_draw_char on a synthetic 2-glyph font.                          */
/* ------------------------------------------------------------------ */

static void test_draw_char(void)
{
    const char *t = "draw_char";
    /* One small font "sheet": widths[glyph], data-offset hi/lo tables,
     * then the glyph bit data, all addressed relative to a single base
     * pointer per gfx.h's gc0e0/gc0de/gc0e2/gc0e4/gc0e6 contract. */
    static uint8_t sheet[64];
    memset(sheet, 0, sizeof sheet);
    /* Layout (all offsets relative to `sheet`):
     *   [0]   width table (2 glyphs)
     *   [4]   data-offset low byte table
     *   [8]   data-offset high byte table
     *   [16]  glyph bit data area (gc0de) */
    sheet[0] = 10; /* glyph 0 width = 10 (needs 2 source bytes/row) */
    sheet[1] = 3;  /* glyph 1 width = 3 */
    sheet[4] = 0;  sheet[8] = 0;   /* glyph0 data offset (from gc0de) = 0 */
    sheet[5] = 3;  sheet[9] = 0;   /* glyph1 data offset (from gc0de) = 3 */

    gc0e0 = sheet;
    gc0e4 = 0;   /* width table offset */
    gc0e6 = 4;   /* data-offset low byte table */
    gc0e2 = 8;   /* data-offset high byte table */
    gc0de = 16;  /* glyph bit area */
    dialog_line_height = 2;

    /* glyph 0: width 10, 2 rows, 2 bytes/row (ceil(10/8)=2). Row0 bits:
     * 1100000000 (top 10 of byte0=0xC0,byte1 top2 bits=00 -> 0x00).
     * Row1 bits: 0011110000. */
    uint8_t *g0 = sheet + 16;
    g0[0] = 0xC0; g0[1] = 0x00;  /* row0 */
    g0[2] = 0x3C; g0[3] = 0x00;  /* row1 */

    fb_zero();
    result = 0x11; /* symmetric colour: both nibbles = 1 */
    dos_int adv = gfx_draw_char(0, 0, 0);
    check(t, adv == 10, "advance width for glyph0 should equal its table width");

    /* Row0: pixels 0,1 set (bits 1,1 then zeros). Even-x start -> pixel k
     * parity: k=0 high nibble byte0, k=1 low nibble byte0. */
    check(t, gfx_get_pixel(0, 0) == 1, "row0 pixel0 should be painted");
    check(t, gfx_get_pixel(1, 0) == 1, "row0 pixel1 should be painted");
    for (int c = 2; c < 10; c++) {
        char msg[64];
        snprintf(msg, sizeof msg, "row0 pixel%d should be unpainted", c);
        check(t, gfx_get_pixel(c, 0) == 0, msg);
    }
    /* Row1: bits 00111100 00 -> pixels 2,3,4,5 set. */
    for (int c = 0; c < 10; c++) {
        int expect = (c >= 2 && c <= 5) ? 1 : 0;
        dos_int got = gfx_get_pixel(c, 1);
        if (got != expect) {
            fprintf(stderr, "test_gfx: %s: row1 pixel%d got=%d expect=%d\n", t, c, (int)got, expect);
            g_failures++;
        }
    }

    /* glyph 1: width 3, odd x start (x=5) to exercise the low-nibble-first
     * path; 1 row for simplicity. */
    dialog_line_height = 1;
    uint8_t *g1 = sheet + 16 + 3;
    g1[0] = 0xA0; /* top 3 bits: 1,0,1 */
    fb_zero();
    result = 0x11; /* symmetric colour: both nibbles = 1 */
    adv = gfx_draw_char(5, 0, 1);
    check(t, adv == 3, "advance width for glyph1 should equal its table width");
    check(t, gfx_get_pixel(5, 0) == 1, "odd-start glyph pixel0 should be painted");
    check(t, gfx_get_pixel(6, 0) == 0, "odd-start glyph pixel1 should be unpainted");
    check(t, gfx_get_pixel(7, 0) == 1, "odd-start glyph pixel2 should be painted");
}

/* ------------------------------------------------------------------ */
/* gfx_copy_rect: transparency (zero nibble = see-through) and         */
/* clipping against g94/g96 (rows) and g98/g9a (byte columns).         */
/* ------------------------------------------------------------------ */

static void test_copy_rect_transparency(void)
{
    const char *t = "copy_rect_transparency";
    /* bitmap header: [0x20]=bytesPerRow, [0x21]=rows, data at [0x22]. */
    static uint8_t bmp[0x22 + 4 * 2];
    memset(bmp, 0, sizeof bmp);
    bmp[0x20] = 2; /* bytes per row -> 4 pixel columns */
    bmp[0x21] = 2; /* rows */
    /* row0: 0x10,0x02 -> nibbles [1,0,0,2]; row1: 0x00,0x30 -> [0,0,3,0] */
    bmp[0x22 + 0] = 0x10; bmp[0x22 + 1] = 0x02;
    bmp[0x22 + 2] = 0x00; bmp[0x22 + 3] = 0x30;

    g94 = -1; g96 = 500; g98 = -1; g9a = 500; /* effectively unclipped */

    fb_zero();
    /* pre-fill destination with a known nonzero pattern so transparency
     * (zero source nibble keeps dest) is observable. */
    result = 0x99;
    gfx_clear_rect(0, 0, 8, 2);

    gbc = 0;
    gfx_copy_rect(0, 0, bmp, 0);

    dos_int expect_row0[4] = { 1, 9, 9, 2 };
    dos_int expect_row1[4] = { 9, 9, 3, 9 };
    for (int c = 0; c < 4; c++) {
        dos_int got0 = gfx_get_pixel(c, 0);
        dos_int got1 = gfx_get_pixel(c, 1);
        if (got0 != expect_row0[c]) {
            fprintf(stderr, "test_gfx: %s: row0 col%d got=%d expect=%d\n", t, c, (int)got0, (int)expect_row0[c]);
            g_failures++;
        }
        if (got1 != expect_row1[c]) {
            fprintf(stderr, "test_gfx: %s: row1 col%d got=%d expect=%d\n", t, c, (int)got1, (int)expect_row1[c]);
            g_failures++;
        }
    }
}

static void test_copy_rect_clipping(void)
{
    const char *t = "copy_rect_clipping";
    static uint8_t bmp[0x22 + 4 * 4];
    memset(bmp, 0, sizeof bmp);
    bmp[0x20] = 4; /* 4 bytes/row -> 8 columns */
    bmp[0x21] = 4; /* 4 rows */
    for (int i = 0; i < 16; i++) bmp[0x22 + i] = 0x11; /* fully opaque, colour 1 */

    /* Clip to rows [1,2] (inclusive, via g94/g96) and byte-columns [1,2]
     * (inclusive, via g98/g9a), drawn at (x=0,y=0) i.e. byte column 0. */
    g94 = 1; g96 = 2;
    g98 = 1; g9a = 2;

    fb_zero();
    gbc = 0;
    gfx_copy_rect(0, 0, bmp, 0);

    /* Rows 0 and 3 must be untouched; rows 1-2 touched only in byte
     * columns 1-2 (pixel columns 2-5). */
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 8; c++) {
            int in_row = (r == 1 || r == 2);
            int in_col = (c >= 2 && c <= 5);
            dos_int expect = (in_row && in_col) ? 1 : 0;
            dos_int got = gfx_get_pixel(c, r);
            if (got != expect) {
                fprintf(stderr, "test_gfx: %s: (%d,%d) got=%d expect=%d\n", t, c, r, (int)got, (int)expect);
                g_failures++;
            }
        }
    }

    g94 = -1; g96 = 500; g98 = -1; g9a = 500; /* restore permissive clip for later tests */
}

/* ------------------------------------------------------------------ */
/* gfx_box present transform on a known row.                           */
/* ------------------------------------------------------------------ */

static void test_box_transform(void)
{
    const char *t = "box_transform";
    fb_zero();
    /* Row 0, byte columns 0..1 (4 pixels): nibbles a,b,c,d. */
    uint8_t *row0 = gfx_framebuffer();
    row0[0] = 0x12; /* pixels 0,1 = 1,2 */
    row0[1] = 0x34; /* pixels 2,3 = 3,4 */

    gfx_box(0, 0, 4, 1);

    /* Per the derivation: for source bytes b0,b1 the 4 VRAM bytes are
     * [b0, (b0<<4|b1>>4)&0xFF, b1, (b1<<4|b0>>4)&0xFF]. */
    uint8_t b0 = 0x12, b1 = 0x34;
    uint8_t expect[4] = {
        b0,
        (uint8_t)((b0 << 4) | (b1 >> 4)),
        b1,
        (uint8_t)((b1 << 4) | (b0 >> 4))
    };
    for (int i = 0; i < 4; i++) {
        uint8_t got = gfx_vram[i];
        if (got != expect[i]) {
            fprintf(stderr, "test_gfx: %s: vram[%d] got=0x%02x expect=0x%02x\n", t, i, got, expect[i]);
            g_failures++;
        }
    }

    uint32_t gen_before = gfx_vram_generation;
    gfx_box(0, 0, 4, 1);
    check(t, gfx_vram_generation == gen_before + 1, "gfx_vram_generation should increment once per gfx_box call");
}

/* ==================================================================== */
/* VGA driver (display_mode 5) first-principles checks.  Caller must set */
/* display_mode = 5 and re-init the framebuffer before this section, and */
/* restore display_mode afterwards (see run_vga_tests() / main()).       */
/*                                                                       */
/* Every helper above (fb_zero/fb_seed/ref_paint_span/ref_paint_vspan/   */
/* rects_equal*) is mode-agnostic -- they only call gfx_get_pixel/       */
/* gfx_set_pixel/gfx_framebuffer()/gfx_row_bytes(), so they are reused   */
/* as-is.  Colours below use the FULL byte range (not just 0..15) to     */
/* prove the VGA driver really writes whole bytes, not nibbles; expected */
/* values are therefore NOT masked to 0x0F the way the mode-4 checks     */
/* (nibble-precision) are.                                               */
/* ==================================================================== */

static void vga_test_bar(void)
{
    const char *t = "vga_bar";
    for (dos_int x = 0; x < 4; x++) {
        for (dos_int n = 1; n <= 5; n++) {
            fb_zero();
            result = 0xC7;
            gfx_bar(x, 0, n);
            ref_paint_span(x, 1, n, 0xC7);
            if (!rects_equal(x, 0, x, 1, n, 1)) {
                fprintf(stderr, "test_gfx: %s: mismatch at x=%d n=%d\n", t, (int)x, (int)n);
                g_failures++;
            }
            if (x > 0 && gfx_get_pixel((dos_int)(x - 1), 0) != 0) fail(t, "bled left of span");
            if (gfx_get_pixel((dos_int)(x + n), 0) != 0) fail(t, "bled right of span");
        }
    }
}

static void vga_test_vline(void)
{
    const char *t = "vga_vline";
    for (dos_int x = 0; x < 4; x++) {
        for (dos_int n = 1; n <= 5; n++) {
            fb_zero();
            result = 0xC7;
            gfx_vline(x, 0, n);
            ref_paint_vspan((dos_int)(x + 20), 0, n, 0xC7);
            if (!rects_equal(x, 0, (dos_int)(x + 20), 0, 1, n)) {
                fprintf(stderr, "test_gfx: %s: mismatch at x=%d n=%d\n", t, (int)x, (int)n);
                g_failures++;
            }
            if (gfx_get_pixel(x, n) != 0) fail(t, "bled below span");
        }
    }
}

static void vga_test_clear_rect(void)
{
    const char *t = "vga_clear_rect";
    for (int iter = 0; iter < 40; iter++) {
        dos_int x = (dos_int)(next_rand() % 40);
        dos_int y = (dos_int)(next_rand() % 30);
        dos_int w = (dos_int)(1 + next_rand() % 20);
        dos_int h = (dos_int)(1 + next_rand() % 8);
        dos_int x2 = (dos_int)(x + 100);

        fb_zero();
        result = 0x9A;
        gfx_clear_rect(x, y, w, h);
        for (dos_int r = 0; r < h; r++) ref_paint_span(x2, (dos_int)(y + r), w, 0x9A);

        if (!rects_equal(x, y, x2, y, w, h)) {
            fprintf(stderr, "test_gfx: %s: mismatch iter=%d x=%d y=%d w=%d h=%d\n",
                    t, iter, (int)x, (int)y, (int)w, (int)h);
            g_failures++;
        }
    }
}

/* gfx_fill_rect XORs 0x0F into every byte, literally, in BOTH drivers
 * (docs/portable/reference/AE000_002-vga-runtime.lst 04B8 `mov al,0xf`).
 * Unlike the mode-4 check, the expected value is NOT masked to 0x0F: VGA
 * pixels are full bytes, so XORing the low nibble of an arbitrary seeded
 * byte can produce any value in 0..255, not just 0..15. */
static void vga_test_fill_rect(void)
{
    const char *t = "vga_fill_rect";
    for (int iter = 0; iter < 30; iter++) {
        dos_int x = (dos_int)(next_rand() % 40);
        dos_int y = (dos_int)(next_rand() % 30);
        dos_int w = (dos_int)(1 + next_rand() % 20);
        dos_int h = (dos_int)(1 + next_rand() % 8);

        fb_seed(0xFACADEu + (uint32_t)iter);
        uint8_t snapshot[32 * 16];
        for (dos_int r = 0; r < h; r++)
            for (dos_int c = 0; c < w; c++)
                snapshot[r * 32 + c] = (uint8_t)gfx_get_pixel((dos_int)(x + c), (dos_int)(y + r));

        gfx_fill_rect(x, y, w, h);
        for (dos_int r = 0; r < h; r++) {
            for (dos_int c = 0; c < w; c++) {
                dos_int expect = (dos_int)(uint8_t)(snapshot[r * 32 + c] ^ 0x0Fu);
                dos_int got = gfx_get_pixel((dos_int)(x + c), (dos_int)(y + r));
                if (got != expect) {
                    fprintf(stderr, "test_gfx: %s: xor mismatch iter=%d (%d,%d) got=%d want=%d\n",
                            t, iter, (int)c, (int)r, (int)got, (int)expect);
                    g_failures++;
                }
            }
        }
        gfx_fill_rect(x, y, w, h); /* toggle back */
        for (dos_int r = 0; r < h; r++) {
            for (dos_int c = 0; c < w; c++) {
                if (gfx_get_pixel((dos_int)(x + c), (dos_int)(y + r)) != snapshot[r * 32 + c]) {
                    fail(t, "double-toggle didn't restore");
                }
            }
        }
    }
}

static void vga_test_save_restore(void)
{
    const char *t = "vga_save_restore";
    dos_int x = 3, y = 5, w = 9, h = 6;
    fb_seed(4200);
    uint8_t buf[4 + 64 * 32];
    gfx_save_rect(x, y, w, h, buf);
    check(t, dos_rd16(buf) == (uint16_t)w, "save header width should equal w (no halving in VGA mode)");
    check(t, dos_rd16(buf + 2) == (uint16_t)h, "save header height should equal h");

    dos_int shadow_x = 200;
    for (dos_int r = 0; r < h; r++)
        for (dos_int c = 0; c < w; c++) {
            dos_int v = gfx_get_pixel((dos_int)(x + c), (dos_int)(y + r));
            dos_int saved = result;
            result = v;
            gfx_set_pixel((dos_int)(shadow_x + c), (dos_int)(y + r));
            result = saved;
        }

    result = 0x00;
    gfx_clear_rect(x, y, w, h);
    check(t, rects_equal(x, y, shadow_x, y, w, h) == 0, "corruption step didn't change anything (test bug)");

    gfx_restore_rect(x, y, buf);
    if (!rects_equal(x, y, shadow_x, y, w, h)) {
        fail(t, "restored region does not match pre-corruption snapshot");
    }
}

static void vga_test_wipe_rect(void)
{
    const char *t = "vga_wipe_rect";
    dos_int sx = 2, sy = 4, w = 10, h = 7, dx = 90, dy = 20;
    fb_seed(700);
    gbc = 0;
    gfx_wipe_rect(sx, sy, w, h, dx, dy);
    if (!rects_equal(sx, sy, dx, dy, w, h)) {
        fail(t, "destination does not match source after wipe");
    }
}

static void vga_test_wipe_rect_dirty_queue(void)
{
    const char *t = "vga_wipe_rect_dirty_queue";
    fb_seed(100);
    gbc = 1;
    uint8_t queue[64];
    uint8_t *saved_queue_ptr = rect_queue_write_ptr;
    memset(queue, 0xAA, sizeof queue);
    rect_queue_write_ptr = queue;
    gfx_wipe_rect(0, 0, 8, 3, 10, 5); /* dy=5 < 0xC8, should append (HALF units, same as mode 4) */
    dos_uint w1 = dos_rd16(queue);
    dos_uint w2 = dos_rd16(queue + 2);
    check(t, (w1 >> 8) == 5, "record y byte wrong");
    check(t, (w1 & 0xFF) == (10 >> 1), "record x/2 byte wrong");
    check(t, (w2 >> 8) == 3, "record h byte wrong");
    check(t, (w2 & 0xFF) == (8 >> 1), "record w/2 byte wrong");
    check(t, rect_queue_write_ptr == queue + 4, "write pointer did not advance by 4");
    rect_queue_write_ptr = saved_queue_ptr;
    gbc = 0;
}

static void vga_test_flip_h(void)
{
    fb_seed(990);
    dos_int sx = 4, sy = 2, w = 12, h = 5, dx = 60, dy = 30;
    gfx_copy_rect_flip_h(sx, sy, w, h, dx, dy);
    if (!rects_equal_mirrored_h(sx, sy, dx, dy, w, h)) {
        fail("vga_flip_h", "destination is not a horizontal mirror of the source");
    }
}

static void vga_test_flip_v(void)
{
    fb_seed(1230);
    dos_int sx = 4, sy = 2, w = 8, h = 6, dx = 60, dy = 40;
    gfx_copy_rect_flip_v(sx, sy, w, h, dx, dy);
    if (!rects_equal_mirrored_v(sx, sy, dx, dy, w, h)) {
        fail("vga_flip_v", "destination is not a vertical mirror of the source");
    }
}

static void vga_test_flip_hv(void)
{
    fb_seed(550);
    dos_int sx = 4, sy = 2, w = 8, h = 6, dx = 60, dy = 60;
    gfx_copy_rect_flip_hv(sx, sy, w, h, dx, dy);
    for (dos_int r = 0; r < h; r++) {
        for (dos_int c = 0; c < w; c++) {
            dos_int a = gfx_get_pixel((dos_int)(sx + c), (dos_int)(sy + r));
            dos_int b = gfx_get_pixel((dos_int)(dx + w - 1 - c), (dos_int)(dy + h - 1 - r));
            if (a != b) {
                fail("vga_flip_hv", "destination is not a 180-degree mirror of the source");
                return;
            }
        }
    }
}

/* gfx_copy_rect_split turns h SOURCE rows into h DESTINATION columns:
 * source row r (0..h-1), columns sx..sx+w-1, lands at dest column
 * dx+h-1-r, rows dy..dy+w-1 (docs/portable/reference/AE000_002-vga-runtime.lst
 * 0694-06DA). */
static void vga_test_split(void)
{
    const char *t = "vga_split";
    dos_int sx = 2, sy = 10, w = 6, h = 8, dx = 150, dy = 100;
    fb_seed(3333);
    gfx_copy_rect_split(sx, sy, w, h, dx, dy);
    for (dos_int r = 0; r < h; r++) {
        for (dos_int k = 0; k < w; k++) {
            dos_int srcv = gfx_get_pixel((dos_int)(sx + k), (dos_int)(sy + r));
            dos_int dstv = gfx_get_pixel((dos_int)(dx + h - 1 - r), (dos_int)(dy + k));
            if (srcv != dstv) {
                fprintf(stderr, "test_gfx: %s: mismatch r=%d k=%d src=%d dst=%d\n",
                        t, (int)r, (int)k, (int)srcv, (int)dstv);
                g_failures++;
                return;
            }
        }
    }
}

/* gfx_copy_rect_split_flip_v: same transpose, vertically flipped: source
 * row r lands at dest column dx+r, rows dy+w-1 down to dy (0x6DB-0x72B). */
static void vga_test_split_flip_v(void)
{
    const char *t = "vga_split_flip_v";
    dos_int sx = 2, sy = 10, w = 6, h = 8, dx = 200, dy = 100;
    fb_seed(4444);
    gfx_copy_rect_split_flip_v(sx, sy, w, h, dx, dy);
    for (dos_int r = 0; r < h; r++) {
        for (dos_int k = 0; k < w; k++) {
            dos_int srcv = gfx_get_pixel((dos_int)(sx + k), (dos_int)(sy + r));
            dos_int dstv = gfx_get_pixel((dos_int)(dx + r), (dos_int)(dy + w - 1 - k));
            if (srcv != dstv) {
                fprintf(stderr, "test_gfx: %s: mismatch r=%d k=%d src=%d dst=%d\n",
                        t, (int)r, (int)k, (int)srcv, (int)dstv);
                g_failures++;
                return;
            }
        }
    }
}

/* gfx_draw_char: same font-state contract as the planar driver, but every
 * painted pixel gets the FULL colour byte (result's low byte), not a
 * nibble, and gfx_draw_char's return value is the raw width (no >>1). */
static void vga_test_draw_char(void)
{
    const char *t = "vga_draw_char";
    static uint8_t sheet[64];
    memset(sheet, 0, sizeof sheet);
    sheet[0] = 10; /* glyph 0 width = 10 (needs 2 source bytes/row) */
    sheet[1] = 3;  /* glyph 1 width = 3 */
    sheet[4] = 0;  sheet[8] = 0;   /* glyph0 data offset (from gc0de) = 0 */
    sheet[5] = 3;  sheet[9] = 0;   /* glyph1 data offset (from gc0de) = 3 */

    gc0e0 = sheet;
    gc0e4 = 0;
    gc0e6 = 4;
    gc0e2 = 8;
    gc0de = 16;
    dialog_line_height = 2;

    uint8_t *g0 = sheet + 16;
    g0[0] = 0xC0; g0[1] = 0x00;  /* row0: pixels 0,1 set */
    g0[2] = 0x3C; g0[3] = 0x00;  /* row1: pixels 2,3,4,5 set */

    fb_zero();
    result = 0xAB; /* full byte colour, deliberately outside 0..15 */
    dos_int adv = gfx_draw_char(0, 0, 0);
    check(t, adv == 10, "advance should equal the raw glyph width (no >>1, unlike planar)");

    check(t, gfx_get_pixel(0, 0) == 0xAB, "row0 pixel0 should carry the full colour byte");
    check(t, gfx_get_pixel(1, 0) == 0xAB, "row0 pixel1 should carry the full colour byte");
    for (int c = 2; c < 10; c++) {
        if (gfx_get_pixel(c, 0) != 0) fail(t, "row0 tail pixel should be unpainted (0)");
    }
    for (int c = 0; c < 10; c++) {
        int expect = (c >= 2 && c <= 5) ? 0xAB : 0;
        dos_int got = gfx_get_pixel(c, 1);
        if (got != expect) {
            fprintf(stderr, "test_gfx: %s: row1 pixel%d got=%d expect=%d\n", t, c, (int)got, expect);
            g_failures++;
        }
    }

    dialog_line_height = 1;
    uint8_t *g1 = sheet + 16 + 3;
    g1[0] = 0xA0; /* top 3 bits: 1,0,1 */
    fb_zero();
    result = 0xAB;
    adv = gfx_draw_char(5, 0, 1);
    check(t, adv == 3, "advance width for glyph1 should equal its table width");
    check(t, gfx_get_pixel(5, 0) == 0xAB, "glyph1 pixel0 should be painted");
    check(t, gfx_get_pixel(6, 0) == 0, "glyph1 pixel1 should be unpainted");
    check(t, gfx_get_pixel(7, 0) == 0xAB, "glyph1 pixel2 should be painted");

    gc0e0 = NULL; gc0de = gc0e2 = gc0e4 = gc0e6 = 0; dialog_line_height = 0;
}

/* gfx_blit_bitmap: table = bitmap+0x10 (16 distinct bytes here, so hi/lo
 * nibble -> table lookup is unambiguous), header at +0x20, data at +0x22;
 * dirty record still HALF units. */
static void vga_test_blit_bitmap(void)
{
    const char *t = "vga_blit_bitmap";
    static uint8_t bmp[0x22 + 3 * 2]; /* 2 bytes/row (4 pixels), 3 rows */
    memset(bmp, 0, sizeof bmp);
    for (int i = 0; i < 16; i++) bmp[0x10 + i] = (uint8_t)(0x40 + i); /* VGA table: table[n] = 0x40+n */
    bmp[0x20] = 2; /* packed bytes/row */
    bmp[0x21] = 3; /* rows */
    bmp[0x22 + 0] = 0x12; bmp[0x22 + 1] = 0x34; /* row0: nibbles 1,2,3,4 */
    bmp[0x22 + 2] = 0x56; bmp[0x22 + 3] = 0x78; /* row1 */
    bmp[0x22 + 4] = 0x9A; bmp[0x22 + 5] = 0xBC; /* row2 */

    fb_zero();
    gbc = 1;
    uint8_t queue[64];
    uint8_t *saved_queue_ptr = rect_queue_write_ptr;
    memset(queue, 0, sizeof queue);
    rect_queue_write_ptr = queue;

    gfx_blit_bitmap(10, 30, bmp);

    static const uint8_t nibbles[3][4] = { {1,2,3,4}, {5,6,7,8}, {9,0xA,0xB,0xC} };
    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 4; c++) {
            dos_int got = gfx_get_pixel((dos_int)(10 + c), (dos_int)(30 + r));
            dos_int want = (dos_int)(0x40 + nibbles[r][c]);
            if (got != want) {
                fprintf(stderr, "test_gfx: %s: (%d,%d) got=%d want=%d\n", t, c, r, (int)got, (int)want);
                g_failures++;
            }
        }
    }

    dos_uint w1 = dos_rd16(queue);
    dos_uint w2 = dos_rd16(queue + 2);
    check(t, (w1 >> 8) == 30, "dirty record y wrong");
    check(t, (w1 & 0xFF) == (10 >> 1), "dirty record x/2 wrong");
    check(t, (w2 >> 8) == 3, "dirty record rows wrong");
    check(t, (w2 & 0xFF) == 2, "dirty record packed_bytes wrong");

    rect_queue_write_ptr = saved_queue_ptr;
    gbc = 0;
}

/* gfx_copy_rect: transparency (zero nibble = see-through, through the
 * table) and row/column clipping, forward and flip=1 (mirrored) paths. */
static void vga_test_copy_rect(void)
{
    const char *t = "vga_copy_rect_transparency";
    static uint8_t bmp[0x22 + 4 * 2];
    memset(bmp, 0, sizeof bmp);
    for (int i = 0; i < 16; i++) bmp[0x10 + i] = (uint8_t)(0x50 + i); /* identity-ish table */
    bmp[0x20] = 2; /* bytes per row -> 4 pixel columns */
    bmp[0x21] = 2; /* rows */
    /* row0: 0x10,0x02 -> nibbles [1,0,0,2]; row1: 0x00,0x30 -> [0,0,3,0] */
    bmp[0x22 + 0] = 0x10; bmp[0x22 + 1] = 0x02;
    bmp[0x22 + 2] = 0x00; bmp[0x22 + 3] = 0x30;

    g94 = -1; g96 = 500; g98 = -1; g9a = 500;

    fb_zero();
    result = 0x99;
    gfx_clear_rect(0, 0, 8, 2); /* known nonzero background so transparency is observable */

    gbc = 0;
    gfx_copy_rect(0, 0, bmp, 0);

    dos_int expect_row0[4] = { 0x51, 0x99, 0x99, 0x52 };
    dos_int expect_row1[4] = { 0x99, 0x99, 0x53, 0x99 };
    for (int c = 0; c < 4; c++) {
        dos_int got0 = gfx_get_pixel(c, 0);
        dos_int got1 = gfx_get_pixel(c, 1);
        if (got0 != expect_row0[c]) {
            fprintf(stderr, "test_gfx: %s: row0 col%d got=%d expect=%d\n", t, c, (int)got0, (int)expect_row0[c]);
            g_failures++;
        }
        if (got1 != expect_row1[c]) {
            fprintf(stderr, "test_gfx: %s: row1 col%d got=%d expect=%d\n", t, c, (int)got1, (int)expect_row1[c]);
            g_failures++;
        }
    }

    /* Row/column clipping: 4 rows x 4 packed bytes (8 pixel columns), fully
     * opaque colour 1, clipped to rows [1,2] and PACKED columns [1,2]
     * (pixel columns 2..5) via g94/g96/g98/g9a. */
    {
        const char *tc = "vga_copy_rect_clipping";
        static uint8_t bmp2[0x22 + 4 * 4];
        memset(bmp2, 0, sizeof bmp2);
        for (int i = 0; i < 16; i++) bmp2[0x10 + i] = (uint8_t)i; /* identity table: keeps nibble==value */
        bmp2[0x20] = 4;
        bmp2[0x21] = 4;
        for (int i = 0; i < 16; i++) bmp2[0x22 + i] = 0x11; /* opaque, nibble 1 both halves */

        g94 = 1; g96 = 2;
        g98 = 1; g9a = 2;

        fb_zero();
        gbc = 0;
        gfx_copy_rect(0, 0, bmp2, 0);

        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 8; c++) {
                int in_row = (r == 1 || r == 2);
                int in_col = (c >= 2 && c <= 5);
                dos_int expect = (in_row && in_col) ? 1 : 0;
                dos_int got = gfx_get_pixel(c, r);
                if (got != expect) {
                    fprintf(stderr, "test_gfx: %s: (%d,%d) got=%d expect=%d\n", tc, c, r, (int)got, (int)expect);
                    g_failures++;
                }
            }
        }
        g94 = -1; g96 = 500; g98 = -1; g9a = 500;
    }

    /* flip=1: horizontally mirrored draw of the same fully-opaque 4x4
     * packed bitmap; column c of the source should land at (w_px-1-c). */
    {
        const char *tf = "vga_copy_rect_flip";
        static uint8_t bmp3[0x22 + 2 * 1];
        memset(bmp3, 0, sizeof bmp3);
        for (int i = 0; i < 16; i++) bmp3[0x10 + i] = (uint8_t)(0x60 + i);
        bmp3[0x20] = 2; /* bytes per row -> 4 pixel columns */
        bmp3[0x21] = 1; /* 1 row */
        bmp3[0x22 + 0] = 0x12; /* nibbles [1,2] (packed byte 0) */
        bmp3[0x22 + 1] = 0x34; /* nibbles [3,4] (packed byte 1) */
        /* unmirrored pixel order (left to right): 1,2,3,4 -> table 0x61,0x62,0x63,0x64 */

        fb_zero();
        gbc = 0;
        gfx_copy_rect(20, 0, bmp3, 1);

        dos_int want[4] = { 0x64, 0x63, 0x62, 0x61 }; /* mirrored: rightmost source pixel first */
        for (int c = 0; c < 4; c++) {
            dos_int got = gfx_get_pixel((dos_int)(20 + c), 0);
            if (got != want[c]) {
                fprintf(stderr, "test_gfx: %s: col%d got=%d want=%d\n", tf, c, (int)got, (int)want[c]);
                g_failures++;
            }
        }
    }

    /* A left-facing player can be drawn at x=-16 while it is approaching a
     * room transition.  The original VGA routine wraps the two 16-bit BX
     * additions before deriving the mirrored right edge, so the first four
     * packed source bytes remain visible at screen pixels 8..15.  This is a
     * real-coordinate regression test, rather than a live-vs-composed
     * comparison that could reproduce the same clipping error twice. */
    {
        const char *te = "vga_copy_rect_left_edge_flip";
        static uint8_t edge[0x22 + 16];
        memset(edge, 0, sizeof edge);
        edge[0x10 + 1] = 0xA1;
        edge[0x20] = 16; /* 32 pixels, 16 packed bytes per row */
        edge[0x21] = 1;
        for (int i = 0; i < 4; i++) edge[0x22 + i] = 0x11;

        g94 = 0; g96 = 0; g98 = 4; g9a = 155;
        fb_zero();
        gbc = 0;
        gfx_copy_rect(-16, 0, edge, 1);
        for (int x = 0; x < 8; x++)
            check(te, gfx_get_pixel((dos_int)x, 0) == 0, "bled into clipped-off pixels");
        for (int x = 8; x < 16; x++)
            check(te, gfx_get_pixel((dos_int)x, 0) == 0xA1, "partial left-edge sprite was clipped away");
    }
}

/* gfx_box (present): for display_mode 5 this is a literal byte-for-byte
 * copy from g3924[y]+x into gfx_vram (no nibble unpacking, unlike the
 * planar driver -- docs/portable/reference/AE000_002-vga-runtime.lst
 * 03DE-0424). */
static void vga_test_box(void)
{
    const char *t = "vga_box";
    fb_zero();
    uint8_t *row0 = gfx_framebuffer();
    for (int i = 0; i < 6; i++) row0[i] = (uint8_t)(0x20 + i);

    gfx_box(0, 0, 6, 1);
    for (int i = 0; i < 6; i++) {
        uint8_t got = gfx_vram[i];
        uint8_t want = (uint8_t)(0x20 + i);
        if (got != want) {
            fprintf(stderr, "test_gfx: %s: vram[%d] got=0x%02x want=0x%02x\n", t, i, got, want);
            g_failures++;
        }
    }

    uint32_t gen_before = gfx_vram_generation;
    gfx_box(0, 0, 6, 1);
    check(t, gfx_vram_generation == gen_before + 1, "gfx_vram_generation should increment once per gfx_box call");
}

/* Runs the whole VGA section under display_mode = 5 with its own
 * framebuffer, then restores display_mode = 4 and re-inits for whatever
 * runs next (the oracle-fixture replayer switches per case on its own, but
 * main()'s own bookkeeping stays simple if display_mode is mode 4 again
 * whenever a "plain" gfx_framebuffer_init() happens outside that replay). */
static void run_vga_tests(void)
{
    display_mode = 5;
    gfx_framebuffer_init();
    g94 = -1; g96 = 500; g98 = -1; g9a = 500;
    gc0e0 = NULL; gc0de = gc0e2 = gc0e4 = gc0e6 = 0;
    dialog_line_height = 0;
    gbc = 0;

    vga_test_bar();
    vga_test_vline();
    vga_test_clear_rect();
    vga_test_fill_rect();
    vga_test_save_restore();
    vga_test_wipe_rect();
    vga_test_wipe_rect_dirty_queue();
    vga_test_flip_h();
    vga_test_flip_v();
    vga_test_flip_hv();
    vga_test_split();
    vga_test_split_flip_v();
    vga_test_draw_char();
    vga_test_blit_bitmap();
    vga_test_copy_rect();
    vga_test_box();

    display_mode = 4;
    gfx_framebuffer_init();
}

/* ------------------------------------------------------------------ */
/* fixtures/gfx_cases.json loader/replayer (runs only if the file is   */
/* present -- it is produced by a separate agent's oracle harness).    */
/*                                                                     */
/* Observed format (one JSON array of case objects):                   */
/*   {name, seed, state:{result,gbc,g94,g96,g98,g9a},                  */
/*    calls:[{op,args:[...],blob_hex?}],                               */
/*    font?:{blob_hex,gc0de,gc0e2,gc0e4,gc0e6,line_height},            */
/*    expect:{fb_sha256, fb_rows_touched:{"y":"hex"}, dirty_hex,       */
/*             ret, save_hex}}                                         */
/* seed fills the framebuffer via the LCG in fb_seed() above.  Every op */
/* in gfx.h except gfx_box appears in the fixture and is replayed for   */
/* real, including draw_char (font state wired from the case's "font"   */
/* object by load_font_if_present(), all four gc0dX offsets relative to */
/* that object's own blob_hex) and blit_image (widths 4/8/12/16 only -- */
/* see the comment above gfx_blit_image in primitives.c for why other   */
/* widths are absent from the fixture).  A small hand-rolled JSON        */
/* navigator (object/array field lookup only, no general parse tree) is  */
/* used since no JSON library is linked into the portable tree; a        */
/* draw_char case with no "font" object, or any op/arg-shape this        */
/* loader doesn't recognize, is skipped defensively rather than           */
/* crashing the loader.                                                   */
/* ------------------------------------------------------------------ */

static int hex_nibble(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static size_t hex_decode(const char *hex, uint8_t *out, size_t out_cap)
{
    size_t n = 0;
    while (hex[0] && hex[1] && n < out_cap) {
        int hi = hex_nibble(hex[0]);
        int lo = hex_nibble(hex[1]);
        if (hi < 0 || lo < 0) break;
        out[n++] = (uint8_t)((hi << 4) | lo);
        hex += 2;
    }
    return n;
}

static void hex_encode(const uint8_t *in, size_t n, char *out, size_t out_cap)
{
    static const char digits[] = "0123456789abcdef";
    size_t i;
    for (i = 0; i < n && i * 2 + 2 < out_cap; i++) {
        out[i * 2]     = digits[in[i] >> 4];
        out[i * 2 + 1] = digits[in[i] & 0x0Fu];
    }
    out[i * 2] = '\0';
}

/* ---- minimal JSON navigator: object/array field lookup only ---- */

static const char *json_skip_ws(const char *p)
{
    while (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t') p++;
    return p;
}

static const char *json_skip_string(const char *p)
{
    if (*p != '"') return p;
    p++;
    while (*p && *p != '"') {
        if (*p == '\\' && p[1]) p++;
        p++;
    }
    if (*p == '"') p++;
    return p;
}

static const char *json_skip_value(const char *p)
{
    p = json_skip_ws(p);
    if (*p == '"') return json_skip_string(p);
    if (*p == '{' || *p == '[') {
        char open = *p;
        char close = (open == '{') ? '}' : ']';
        int depth = 1;
        p++;
        while (*p && depth > 0) {
            if (*p == '"') { p = json_skip_string(p); continue; }
            if (*p == open) depth++;
            else if (*p == close) depth--;
            p++;
        }
        return p;
    }
    while (*p && *p != ',' && *p != '}' && *p != ']' &&
           *p != ' ' && *p != '\n' && *p != '\r' && *p != '\t') {
        p++;
    }
    return p;
}

/* p must point at the object's '{'.  Returns a pointer to the value of
 * `key` at this object's own nesting level, or NULL. */
static const char *json_obj_get(const char *p, const char *key)
{
    size_t keylen;
    if (!p || *p != '{') return NULL;
    keylen = strlen(key);
    p = json_skip_ws(p + 1);
    while (*p && *p != '}') {
        const char *kstart;
        const char *after_key;
        size_t klen;
        if (*p != '"') return NULL;
        kstart = p + 1;
        after_key = json_skip_string(p);
        klen = (size_t)((after_key - 1) - kstart);
        p = json_skip_ws(after_key);
        if (*p == ':') p = json_skip_ws(p + 1);
        if (klen == keylen && strncmp(kstart, key, klen) == 0) {
            return p;
        }
        p = json_skip_ws(json_skip_value(p));
        if (*p == ',') p = json_skip_ws(p + 1);
    }
    return NULL;
}

/* p must point at the array's '['.  Calls cb(element_value, ctx) for
 * each element. */
static void json_arr_foreach(const char *p, void (*cb)(const char *, void *), void *ctx)
{
    if (!p || *p != '[') return;
    p = json_skip_ws(p + 1);
    while (*p && *p != ']') {
        cb(p, ctx);
        p = json_skip_ws(json_skip_value(p));
        if (*p == ',') p = json_skip_ws(p + 1);
    }
}

static long json_as_long(const char *p)
{
    if (!p) return 0;
    return strtol(json_skip_ws(p), NULL, 10);
}

/* "seed" is a full uint32_t; strtol's signed `long` (32-bit even in LP64/
 * LLP64 Windows builds) would clamp instead of wrap for values above
 * LONG_MAX (roughly half of all uint32 seeds), silently seeding the wrong
 * framebuffer pattern.  Use strtoul for it specifically. */
static uint32_t json_as_u32(const char *p)
{
    if (!p) return 0;
    return (uint32_t)strtoul(json_skip_ws(p), NULL, 10);
}

static int json_is_null(const char *p)
{
    return p && p[0] == 'n' && p[1] == 'u' && p[2] == 'l' && p[3] == 'l';
}

static void json_as_str(const char *p, char *out, size_t out_cap)
{
    size_t n = 0;
    if (!p || *p != '"') { out[0] = '\0'; return; }
    p++;
    while (*p && *p != '"' && n + 1 < out_cap) {
        if (*p == '\\' && p[1]) p++;
        out[n++] = *p++;
    }
    out[n] = '\0';
}

/* ---- case replay ---- */

typedef struct { long v[8]; int n; } arg_list;

static void collect_arg_cb(const char *elem, void *ctx)
{
    arg_list *a = (arg_list *)ctx;
    if (a->n < 8) a->v[a->n++] = json_as_long(elem);
}

typedef struct {
    int unsupported;
    int have_ret;
    dos_int last_ret;
    char last_op[32];
    uint8_t last_save[4 + 8192];
} call_ctx;

static void run_one_call(const char *call_obj, void *vctx)
{
    call_ctx *ctx = (call_ctx *)vctx;
    char op[32];
    arg_list args;
    static uint8_t blob[8192];
    size_t blob_len = 0;
    const char *blobp;

    json_as_str(json_obj_get(call_obj, "op"), op, sizeof op);
    strncpy(ctx->last_op, op, sizeof(ctx->last_op) - 1);
    ctx->last_op[sizeof(ctx->last_op) - 1] = '\0';
    args.n = 0;
    json_arr_foreach(json_obj_get(call_obj, "args"), collect_arg_cb, &args);

    blobp = json_obj_get(call_obj, "blob_hex");
    if (blobp && !json_is_null(blobp)) {
        static char hex[16384];
        json_as_str(blobp, hex, sizeof hex);
        blob_len = hex_decode(hex, blob, sizeof blob);
    }

    ctx->have_ret = 0;

    if (strcmp(op, "bar") == 0 && args.n == 3) {
        gfx_bar((dos_int)args.v[0], (dos_int)args.v[1], (dos_int)args.v[2]);
    } else if (strcmp(op, "vline") == 0 && args.n == 3) {
        gfx_vline((dos_int)args.v[0], (dos_int)args.v[1], (dos_int)args.v[2]);
    } else if (strcmp(op, "clear_rect") == 0 && args.n == 4) {
        gfx_clear_rect((dos_int)args.v[0], (dos_int)args.v[1], (dos_int)args.v[2], (dos_int)args.v[3]);
    } else if (strcmp(op, "fill_rect") == 0 && args.n == 4) {
        gfx_fill_rect((dos_int)args.v[0], (dos_int)args.v[1], (dos_int)args.v[2], (dos_int)args.v[3]);
    } else if (strcmp(op, "save_rect") == 0 && args.n == 4) {
        memset(ctx->last_save, 0, sizeof ctx->last_save);
        gfx_save_rect((dos_int)args.v[0], (dos_int)args.v[1], (dos_int)args.v[2], (dos_int)args.v[3],
                       ctx->last_save);
    } else if (strcmp(op, "restore_rect") == 0 && args.n == 2 && blob_len > 0) {
        gfx_restore_rect((dos_int)args.v[0], (dos_int)args.v[1], blob);
    } else if (strcmp(op, "wipe_rect") == 0 && args.n == 6) {
        gfx_wipe_rect((dos_int)args.v[0], (dos_int)args.v[1], (dos_int)args.v[2],
                       (dos_int)args.v[3], (dos_int)args.v[4], (dos_int)args.v[5]);
    } else if (strcmp(op, "copy_rect_flip_v") == 0 && args.n == 6) {
        gfx_copy_rect_flip_v((dos_int)args.v[0], (dos_int)args.v[1], (dos_int)args.v[2],
                              (dos_int)args.v[3], (dos_int)args.v[4], (dos_int)args.v[5]);
    } else if (strcmp(op, "copy_rect_flip_h") == 0 && args.n == 6) {
        gfx_copy_rect_flip_h((dos_int)args.v[0], (dos_int)args.v[1], (dos_int)args.v[2],
                              (dos_int)args.v[3], (dos_int)args.v[4], (dos_int)args.v[5]);
    } else if (strcmp(op, "copy_rect_flip_hv") == 0 && args.n == 6) {
        gfx_copy_rect_flip_hv((dos_int)args.v[0], (dos_int)args.v[1], (dos_int)args.v[2],
                               (dos_int)args.v[3], (dos_int)args.v[4], (dos_int)args.v[5]);
    } else if (strcmp(op, "copy_rect_split") == 0 && args.n == 6) {
        gfx_copy_rect_split((dos_int)args.v[0], (dos_int)args.v[1], (dos_int)args.v[2],
                             (dos_int)args.v[3], (dos_int)args.v[4], (dos_int)args.v[5]);
    } else if (strcmp(op, "copy_rect_split_flip_v") == 0 && args.n == 6) {
        gfx_copy_rect_split_flip_v((dos_int)args.v[0], (dos_int)args.v[1], (dos_int)args.v[2],
                                    (dos_int)args.v[3], (dos_int)args.v[4], (dos_int)args.v[5]);
    } else if (strcmp(op, "blit_bitmap") == 0 && args.n == 2 && blob_len > 0) {
        gfx_blit_bitmap((dos_int)args.v[0], (dos_int)args.v[1], blob);
    } else if (strcmp(op, "copy_rect") == 0 && args.n == 3 && blob_len > 0) {
        gfx_copy_rect((dos_int)args.v[0], (dos_int)args.v[1], blob, (dos_int)args.v[2]);
    } else if (strcmp(op, "set_pixel") == 0 && args.n == 2) {
        gfx_set_pixel((dos_int)args.v[0], (dos_int)args.v[1]);
    } else if (strcmp(op, "get_pixel") == 0 && args.n == 2) {
        ctx->last_ret = gfx_get_pixel((dos_int)args.v[0], (dos_int)args.v[1]);
        ctx->have_ret = 1;
    } else if (strcmp(op, "draw_char") == 0 && args.n == 3) {
        /* Needs gc0e0/gc0de/gc0e2/gc0e4/gc0e6/dialog_line_height already
         * set from the case's top-level "font" object (run_one_case, run
         * before the calls array is walked). If a draw_char case somehow
         * carries no font object, gc0e0 stays NULL and drawing through it
         * would crash -- skip defensively instead. */
        if (gc0e0 == NULL) {
            ctx->unsupported = 1;
        } else {
            ctx->last_ret = gfx_draw_char((dos_int)args.v[0], (dos_int)args.v[1], (dos_int)args.v[2]);
            ctx->have_ret = 1;
        }
    } else if (strcmp(op, "blit_image") == 0 && args.n == 2 && blob_len > 0) {
        gfx_blit_image((dos_int)args.v[0], (dos_int)args.v[1], blob);
    } else {
        /* Any op/arg-shape this loader doesn't recognize yet. */
        ctx->unsupported = 1;
    }
}

typedef struct { int total, passed, failed, skipped; } case_stats;

/* op_stat keys are "<op>" for mode-4 cases and "<op>[mode5]" for mode-5
 * cases, so the printed table (run_gfx_cases_if_present) reports mode-5
 * pass/fail per op separately from mode-4's, per the coordinator's request. */
typedef struct { char op[40]; int passed, failed, skipped; } op_stat;
static op_stat g_op_stats[48];
static int g_op_stat_count = 0;

static op_stat *op_stat_get(const char *op)
{
    int i;
    op_stat *s;
    for (i = 0; i < g_op_stat_count; i++) {
        if (strcmp(g_op_stats[i].op, op) == 0) return &g_op_stats[i];
    }
    if (g_op_stat_count >= (int)(sizeof(g_op_stats) / sizeof(g_op_stats[0]))) return NULL;
    s = &g_op_stats[g_op_stat_count++];
    memset(s, 0, sizeof *s);
    strncpy(s->op, op, sizeof(s->op) - 1);
    return s;
}

/* outcome: 1 = passed, 2 = failed, anything else = skipped.  `mode` is the
 * case's own display_mode (4 or 5): mode-5 outcomes are tracked under a
 * distinct op_stat key so they can be reported separately. */
static void record_outcome(case_stats *st, const char *op, dos_char mode, int outcome)
{
    char key[40];
    op_stat *os;
    if (mode == 5) snprintf(key, sizeof key, "%s[mode5]", op);
    else           snprintf(key, sizeof key, "%s", op);
    os = op_stat_get(key);
    if (outcome == 1) {
        st->passed++;
        if (os) os->passed++;
    } else if (outcome == 2) {
        st->failed++;
        g_failures++;
        if (os) os->failed++;
    } else {
        st->skipped++;
        if (os) os->skipped++;
    }
}

static void report_row_diffs(const char *rows_obj)
{
    const char *p = rows_obj;
    if (!p || *p != '{') return;
    p = json_skip_ws(p + 1);
    while (*p && *p != '}') {
        const char *kstart = p + 1;
        const char *after_key = json_skip_string(p);
        int rownum = (int)strtol(kstart, NULL, 10);
        char want_row[65];
        p = json_skip_ws(after_key);
        if (*p == ':') p = json_skip_ws(p + 1);
        json_as_str(p, want_row, sizeof want_row);
        if (rownum >= 0 && rownum < GFX_ROWS) {
            char got_row[65];
            sha256_hex(g3924[rownum], gfx_row_bytes(), got_row);
            if (strcmp(want_row, got_row) != 0) {
                printf("test_gfx: gfx_cases:   row %d differs\n", rownum);
            }
        }
        p = json_skip_ws(json_skip_value(p));
        if (*p == ',') p = json_skip_ws(p + 1);
    }
}

/* Case-level "font" object (draw_char only): {blob_hex, gc0de, gc0e2,
 * gc0e4, gc0e6, line_height}, all four offsets relative to the blob's own
 * start.  Wires gc0e0/gc0de/gc0e2/gc0e4/gc0e6/dialog_line_height (gfx.h's
 * font state) straight from the fixture instead of a synthetic sheet. */
static uint8_t g_font_buf[8192];

static void load_font_if_present(const char *case_obj)
{
    const char *font = json_obj_get(case_obj, "font");
    if (!font) return;
    {
        static char hex[16384];
        json_as_str(json_obj_get(font, "blob_hex"), hex, sizeof hex);
        hex_decode(hex, g_font_buf, sizeof g_font_buf);
    }
    gc0e0 = g_font_buf;
    gc0de = (dos_uint)json_as_long(json_obj_get(font, "gc0de"));
    gc0e2 = (dos_uint)json_as_long(json_obj_get(font, "gc0e2"));
    gc0e4 = (dos_uint)json_as_long(json_obj_get(font, "gc0e4"));
    gc0e6 = (dos_uint)json_as_long(json_obj_get(font, "gc0e6"));
    dialog_line_height = (dos_int)json_as_long(json_obj_get(font, "line_height"));
}

/* Fixture cases carry an optional "mode" field (4 if absent); switching
 * modes mid-replay needs a fresh gfx_framebuffer_init() (row stride 0xA0 vs
 * 0x140) before the next case's fb_seed().  Tracked with a sentinel so the
 * very first case (whatever mode it asks for) always (re-)initializes,
 * regardless of what mode earlier hand-authored tests left active. */
static int s_fixture_mode_active = -1;

static void ensure_fixture_mode(dos_char m)
{
    if (s_fixture_mode_active == m) return;
    display_mode = m;
    gfx_framebuffer_shutdown();
    gfx_framebuffer_init();
    s_fixture_mode_active = m;
}

/* On a fb_sha256 mismatch, report_row_diffs() already names which rows
 * differ.  The fixture only carries a SHA-256 per row (not raw bytes), so
 * we cannot recover the oracle's exact expected bytes to diff against --
 * the best further diagnostic available is dumping our own computed bytes
 * for the first differing row so a human can compare them against the
 * font bits / expected glyph shape (mode 4) or the .lst (mode 5) by hand. */
static void dump_first_row_bytes(const char *rows_obj)
{
    const char *p = rows_obj;
    if (!p || *p != '{') return;
    p = json_skip_ws(p + 1);
    if (*p == '}') return;
    {
        const char *kstart = p + 1;
        int rownum = (int)strtol(kstart, NULL, 10);
        if (rownum >= 0 && rownum < GFX_ROWS) {
            const uint8_t *row = g3924[rownum];
            int i;
            printf("test_gfx: gfx_cases:   row %d our bytes:", rownum);
            for (i = 0; i < gfx_row_bytes(); i++) {
                if (row[i] != 0) printf(" [%d]=%02x", i, row[i]);
            }
            printf("\n");
        }
    }
}

static void run_one_case(const char *case_obj, void *vstats)
{
    case_stats *st = (case_stats *)vstats;
    char name[64];
    uint32_t seed;
    dos_char case_mode;
    const char *modep;
    const char *state;
    const char *callsp;
    const char *expect;
    call_ctx cctx;
    uint8_t *queue_start;
    char got_sha[65], want_sha[65];

    st->total++;
    json_as_str(json_obj_get(case_obj, "name"), name, sizeof name);

    modep = json_obj_get(case_obj, "mode");
    case_mode = (dos_char)(modep ? json_as_long(modep) : 4);
    ensure_fixture_mode(case_mode);

    seed = json_as_u32(json_obj_get(case_obj, "seed"));
    fb_seed(seed);

    state = json_obj_get(case_obj, "state");
    result = (dos_int)json_as_long(json_obj_get(state, "result"));
    gbc    = (dos_int)json_as_long(json_obj_get(state, "gbc"));
    g94    = (dos_int)json_as_long(json_obj_get(state, "g94"));
    g96    = (dos_int)json_as_long(json_obj_get(state, "g96"));
    g98    = (dos_int)json_as_long(json_obj_get(state, "g98"));
    g9a    = (dos_int)json_as_long(json_obj_get(state, "g9a"));

    load_font_if_present(case_obj);

    memset(&cctx, 0, sizeof cctx);
    queue_start = rect_queue_write_ptr;

    callsp = json_obj_get(case_obj, "calls");
    json_arr_foreach(callsp, run_one_call, &cctx);

    if (cctx.unsupported) {
        record_outcome(st, cctx.last_op[0] ? cctx.last_op : "?", case_mode, 0);
        return;
    }

    expect = json_obj_get(case_obj, "expect");
    json_as_str(json_obj_get(expect, "fb_sha256"), want_sha, sizeof want_sha);
    sha256_hex(gfx_framebuffer(), (size_t)gfx_row_bytes() * GFX_ROWS, got_sha);

    if (strcmp(want_sha, got_sha) != 0) {
        printf("test_gfx: gfx_cases: FAIL '%s' (mode %d): fb_sha256 mismatch\n", name, (int)case_mode);
        report_row_diffs(json_obj_get(expect, "fb_rows_touched"));
        /* Any mode-5 failure gets the same byte dump draw_char failures
         * always got: the coordinator asked for the first differing row's
         * bytes so it can be re-derived from the .lst by hand. */
        if (strcmp(cctx.last_op, "draw_char") == 0 || case_mode == 5) {
            dump_first_row_bytes(json_obj_get(expect, "fb_rows_touched"));
        }
        record_outcome(st, cctx.last_op, case_mode, 2);
        return;
    }

    if (cctx.have_ret) {
        long want_ret = json_as_long(json_obj_get(expect, "ret"));
        if ((long)cctx.last_ret != want_ret) {
            printf("test_gfx: gfx_cases: FAIL '%s': ret mismatch got=%ld want=%ld\n",
                   name, (long)cctx.last_ret, want_ret);
            record_outcome(st, cctx.last_op, case_mode, 2);
            return;
        }
    }

    {
        char want_dirty[256];
        json_as_str(json_obj_get(expect, "dirty_hex"), want_dirty, sizeof want_dirty);
        if (want_dirty[0] != '\0') {
            char got_dirty[256];
            size_t n = (size_t)(rect_queue_write_ptr - queue_start);
            hex_encode(queue_start, n, got_dirty, sizeof got_dirty);
            if (strcmp(want_dirty, got_dirty) != 0) {
                printf("test_gfx: gfx_cases: FAIL '%s': dirty_hex mismatch got=%s want=%s\n",
                       name, got_dirty, want_dirty);
                record_outcome(st, cctx.last_op, case_mode, 2);
                return;
            }
        }
    }

    {
        char want_save[256];
        json_as_str(json_obj_get(expect, "save_hex"), want_save, sizeof want_save);
        if (want_save[0] != '\0') {
            uint16_t bytes = dos_rd16(cctx.last_save);
            uint16_t rows = dos_rd16(cctx.last_save + 2);
            size_t n = 4 + (size_t)bytes * rows;
            char got_save[256];
            if (n <= sizeof cctx.last_save) {
                hex_encode(cctx.last_save, n, got_save, sizeof got_save);
                if (strcmp(want_save, got_save) != 0) {
                    printf("test_gfx: gfx_cases: FAIL '%s': save_hex mismatch\n", name);
                    record_outcome(st, cctx.last_op, case_mode, 2);
                    return;
                }
            }
        }
    }

    record_outcome(st, cctx.last_op, case_mode, 1);
}

static void run_gfx_cases_if_present(void)
{
    const char *fixture_dir = EMPIRES_FIXTURE_DIR;
    char path[1024];
    FILE *f;
    long len;
    char *buf;
    size_t got;
    case_stats st;

    snprintf(path, sizeof path, "%s/gfx_cases.json", fixture_dir);
    f = fopen(path, "rb");
    if (!f) {
        printf("test_gfx: %s not found, skipping oracle-fixture cases\n", path);
        return;
    }

    fseek(f, 0, SEEK_END);
    len = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (len <= 0) { fclose(f); return; }
    buf = (char *)malloc((size_t)len + 1);
    if (!buf) { fclose(f); return; }
    got = fread(buf, 1, (size_t)len, f);
    buf[got] = '\0';
    fclose(f);

    memset(&st, 0, sizeof st);
    g_op_stat_count = 0;
    s_fixture_mode_active = -1;
    json_arr_foreach(json_skip_ws(buf), run_one_case, &st);

    printf("test_gfx: gfx_cases.json: %d case(s): %d passed, %d failed, %d skipped\n",
           st.total, st.passed, st.failed, st.skipped);
    {
        /* Two passes so mode-4 ops (plain key) print before mode-5 ops
         * (key suffixed "[mode5]"), giving a clean, separately reportable
         * mode-5 block regardless of insertion order. */
        int i;
        int m5_total = 0, m5_passed = 0, m5_failed = 0, m5_skipped = 0;
        printf("test_gfx: gfx_cases:   -- mode 4 --\n");
        for (i = 0; i < g_op_stat_count; i++) {
            if (strstr(g_op_stats[i].op, "[mode5]")) continue;
            printf("test_gfx: gfx_cases:   %-24s passed=%d failed=%d skipped=%d\n",
                   g_op_stats[i].op, g_op_stats[i].passed, g_op_stats[i].failed,
                   g_op_stats[i].skipped);
        }
        printf("test_gfx: gfx_cases:   -- mode 5 (VGA) --\n");
        for (i = 0; i < g_op_stat_count; i++) {
            if (!strstr(g_op_stats[i].op, "[mode5]")) continue;
            printf("test_gfx: gfx_cases:   %-24s passed=%d failed=%d skipped=%d\n",
                   g_op_stats[i].op, g_op_stats[i].passed, g_op_stats[i].failed,
                   g_op_stats[i].skipped);
            m5_total += g_op_stats[i].passed + g_op_stats[i].failed + g_op_stats[i].skipped;
            m5_passed += g_op_stats[i].passed;
            m5_failed += g_op_stats[i].failed;
            m5_skipped += g_op_stats[i].skipped;
        }
        printf("test_gfx: gfx_cases: mode 5 (VGA) subtotal: %d case(s): %d passed, %d failed, %d skipped\n",
               m5_total, m5_passed, m5_failed, m5_skipped);
    }

    free(buf);
}

/* ------------------------------------------------------------------ */

int main(void)
{
#ifdef _MSC_VER
    /* Never let a debug-CRT assertion/abort pop a blocking modal dialog in
     * an unattended test run; route it to stderr and a normal abort. */
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
#endif
    display_mode = 4;
    gfx_framebuffer_init();
    g94 = -1; g96 = 500; g98 = -1; g9a = 500;
    gc0e0 = NULL; gc0de = gc0e2 = gc0e4 = gc0e6 = 0;
    dialog_line_height = 0;
    gbc = 0;

    test_bar();
    test_vline();
    test_clear_rect();
    test_fill_rect();
    test_save_restore();
    test_wipe_rect();
    test_wipe_rect_dirty_queue();
    test_flip_h();
    test_flip_v();
    test_flip_hv();
    test_draw_char();
    test_copy_rect_transparency();
    test_copy_rect_clipping();
    test_box_transform();

    run_vga_tests();   /* display_mode = 5 section; leaves display_mode = 4 behind */

    run_gfx_cases_if_present();

    gfx_framebuffer_shutdown();

    if (g_failures == 0) {
        printf("test_gfx: all checks passed\n");
        return 0;
    }
    fprintf(stderr, "test_gfx: %d failure(s)\n", g_failures);
    return 1;
}
