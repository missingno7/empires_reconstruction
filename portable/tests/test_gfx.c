/* test_gfx.c -- self-checking unit tests for portable/gfx (Milestone C).
 *
 * No DOS oracle fixtures exist yet on this branch (fixtures/gfx_cases.json
 * is produced by a separate agent), so these tests are built from first
 * principles: known-input/known-output checks for the nibble-precision
 * primitives, and cross-checks between a primitive under test and an
 * independent "naive" per-pixel reference built from gfx_set_pixel /
 * gfx_get_pixel (themselves a from-scratch transcription of a *different*
 * ASM routine, runtime_f03d2/f03d5, so agreement is a meaningful check on
 * indexing/parity, not a tautology).
 *
 * A defensive loader for fixtures/gfx_cases.json is included and runs
 * automatically once that file exists; until then it prints a note and is
 * skipped without failing the test.
 */
#include "gfx.h"
#include "sha256.h"

#include <stdio.h>
#include <string.h>

#ifdef _MSC_VER
#include <crtdbg.h>
#endif
#include <stdlib.h>

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

static void fb_zero(void)
{
    memset(gfx_framebuffer(), 0, (size_t)GFX_ROW_BYTES * GFX_ROWS);
}

/* Deterministic LCG seed fill, same recurrence the (future) fixture
 * format uses: x = x*1103515245+12345 (mod 2^32), byte = (x>>16)&0xFF,
 * row-major over the whole 488*160 framebuffer. */
static void fb_seed(uint32_t seed)
{
    uint8_t *p = gfx_framebuffer();
    size_t n = (size_t)GFX_ROW_BYTES * GFX_ROWS;
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

/* ------------------------------------------------------------------ */
/* fixtures/gfx_cases.json loader/replayer (runs only if the file is   */
/* present -- it is produced by a separate agent's oracle harness).    */
/*                                                                     */
/* Observed format (one JSON array of case objects):                   */
/*   {name, seed, state:{result,gbc,g94,g96,g98,g9a},                  */
/*    calls:[{op,args:[...],blob_hex?}],                               */
/*    expect:{fb_sha256, fb_rows_touched:{"y":"hex"}, dirty_hex,       */
/*             ret, save_hex}}                                         */
/* seed fills the framebuffer via the LCG in fb_seed() above.  Every    */
/* op in gfx.h except gfx_draw_char/gfx_blit_image/gfx_box appears in   */
/* the fixture and is replayed for real; draw_char needs the real       */
/* historical font resource (not embedded in the fixture, not available */
/* standalone here) and is skipped with a printed note rather than       */
/* failed.  A small hand-rolled JSON navigator (object/array field       */
/* lookup only, no general parse tree) is used since no JSON library     */
/* is linked into the portable tree; unrecognized ops/fields are          */
/* skipped defensively rather than crashing the loader.                   */
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
    } else {
        /* draw_char/blit_image (need resources this loader doesn't have)
         * or any op/arg-shape this loader doesn't recognize yet. */
        ctx->unsupported = 1;
    }
}

typedef struct { int total, passed, failed, skipped; } case_stats;

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
            sha256_hex(g3924[rownum], GFX_ROW_BYTES, got_row);
            if (strcmp(want_row, got_row) != 0) {
                printf("test_gfx: gfx_cases:   row %d differs\n", rownum);
            }
        }
        p = json_skip_ws(json_skip_value(p));
        if (*p == ',') p = json_skip_ws(p + 1);
    }
}

static void run_one_case(const char *case_obj, void *vstats)
{
    case_stats *st = (case_stats *)vstats;
    char name[64];
    uint32_t seed;
    const char *state;
    const char *callsp;
    const char *expect;
    call_ctx cctx;
    uint8_t *queue_start;
    char got_sha[65], want_sha[65];

    st->total++;
    json_as_str(json_obj_get(case_obj, "name"), name, sizeof name);
    seed = json_as_u32(json_obj_get(case_obj, "seed"));
    fb_seed(seed);

    state = json_obj_get(case_obj, "state");
    result = (dos_int)json_as_long(json_obj_get(state, "result"));
    gbc    = (dos_int)json_as_long(json_obj_get(state, "gbc"));
    g94    = (dos_int)json_as_long(json_obj_get(state, "g94"));
    g96    = (dos_int)json_as_long(json_obj_get(state, "g96"));
    g98    = (dos_int)json_as_long(json_obj_get(state, "g98"));
    g9a    = (dos_int)json_as_long(json_obj_get(state, "g9a"));

    memset(&cctx, 0, sizeof cctx);
    queue_start = rect_queue_write_ptr;

    callsp = json_obj_get(case_obj, "calls");
    json_arr_foreach(callsp, run_one_call, &cctx);

    if (cctx.unsupported) {
        st->skipped++;
        return;
    }

    expect = json_obj_get(case_obj, "expect");
    json_as_str(json_obj_get(expect, "fb_sha256"), want_sha, sizeof want_sha);
    sha256_hex(gfx_framebuffer(), (size_t)GFX_ROW_BYTES * GFX_ROWS, got_sha);

    if (strcmp(want_sha, got_sha) != 0) {
        printf("test_gfx: gfx_cases: FAIL '%s': fb_sha256 mismatch\n", name);
        report_row_diffs(json_obj_get(expect, "fb_rows_touched"));
        st->failed++;
        g_failures++;
        return;
    }

    if (cctx.have_ret) {
        long want_ret = json_as_long(json_obj_get(expect, "ret"));
        if ((long)cctx.last_ret != want_ret) {
            printf("test_gfx: gfx_cases: FAIL '%s': ret mismatch got=%ld want=%ld\n",
                   name, (long)cctx.last_ret, want_ret);
            st->failed++;
            g_failures++;
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
                st->failed++;
                g_failures++;
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
                    st->failed++;
                    g_failures++;
                    return;
                }
            }
        }
    }

    st->passed++;
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
    /* Save/restore the ambient clip + font state so later self-checking
     * tests (there are none after this call in main(), but keep the
     * habit) aren't affected. */
    json_arr_foreach(json_skip_ws(buf), run_one_case, &st);

    printf("test_gfx: gfx_cases.json: %d case(s): %d passed, %d failed, %d skipped "
           "(skipped = needs a resource this standalone loader doesn't have, e.g. the real font)\n",
           st.total, st.passed, st.failed, st.skipped);

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

    run_gfx_cases_if_present();

    gfx_framebuffer_shutdown();

    if (g_failures == 0) {
        printf("test_gfx: all checks passed\n");
        return 0;
    }
    fprintf(stderr, "test_gfx: %d failure(s)\n", g_failures);
    return 1;
}
