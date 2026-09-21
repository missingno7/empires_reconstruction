/* test_asm_drawq.c -- unit tests for portable/game/asm_drawq.c
 * (draw_queue_render_list / draw_queue_render / draw_queue_render_highlighted,
 * the port of asm/DRAWQ.ASM).
 *
 * Exercises draw_queue_render_list() directly (the explicit-pointer core;
 * see asm_drawq.h) against a real gfx_framebuffer_init(display_mode=5)
 * framebuffer and the real global g2380, per the module brief.
 *
 * Verification strategy: each of the three sub-bitmaps a record selects
 * (left cap / middle tile / right cap, see asm_drawq.h) is built as a
 * minimal valid VGA bitmap (16-byte colour table + packed_bytes=1,rows=1
 * header + one packed data byte 0x00, i.e. BOTH destination pixels take
 * table[0]) with a distinct marker value in table[0], so this test can
 * count how many marker bytes of each colour land in the framebuffer row
 * instead of re-deriving gfx_copy_rect/gfx_blit_bitmap's own column/clip
 * arithmetic (that arithmetic is portable/gfx's own test surface, not
 * this module's).
 */
#include "game.h"
#include "asm_drawq.h"

#include <stdio.h>
#include <string.h>

static int g_failures = 0;

static void fail(const char *test, const char *what)
{
    fprintf(stderr, "test_asm_drawq: FAIL %s: %s\n", test, what);
    g_failures++;
}

static void check(const char *test, int cond, const char *what)
{
    if (!cond) fail(test, what);
}

/* Flat byte view of g2380 (dos_char[23][130], 2990 bytes) -- same
 * flattening asm_drawq.c itself performs; see that file for why. */
static dos_char *g2380_flat(void) { return &g2380[0][0]; }

/* Writes a minimal 1x1-pixel VGA bitmap (asm_drawq.h's left-cap/middle-
 * tile/right-cap shape) at g2380 flat offset `flat_off`, whose one packed
 * data byte (0x11, i.e. both nibbles == 1) makes BOTH destination pixels
 * come out as `marker` via table[1].  Nibble value 1, not 0: vga_copy_rect
 * (used for the left/right caps) treats a 0 nibble as transparent and
 * skips writing it, while vga_blit_bitmap (used for the middle tile)
 * writes unconditionally -- 0x11/table[1] draws correctly through either
 * primitive, so one bitmap shape serves all three record parts. */
static void write_marker_bitmap(dos_uint flat_off, dos_uchar marker)
{
    dos_char *base = g2380_flat() + flat_off;
    memset(base, 0, 0x10);          /* table[0],[2..15]: unused */
    base[0x11] = (dos_char)marker;  /* table[1] */
    base[0x20] = 1;                 /* header: packed_bytes = 1 */
    base[0x21] = 1;                 /* header: rows = 1 */
    base[0x22] = (dos_char)0x11;    /* one packed data byte: nibbles 1,1 -> table[1] twice */
}

static void set_permissive_clip(void)
{
    g94 = -1; g96 = 500; g98 = -1; g9a = 500; /* effectively unclipped (test_gfx.c convention) */
}

static int count_marker_in_row(dos_int row, dos_uchar marker)
{
    const uint8_t *p = g3924[(dos_uint)row];
    int n = 0, i;
    for (i = 0; i < GFX_ROW_BYTES_VGA; i++)
        if (p[i] == marker) n++;
    return n;
}

static void setup(void)
{
    display_mode = 5;
    gfx_framebuffer_init();
    memset(gfx_framebuffer(), 0, (size_t)gfx_row_bytes() * GFX_ROWS);
    set_permissive_clip();
    gbc = 0; /* skip the dirty-rect append path (portable/gfx's own concern, not this module's) */
    memset(g2380, 0, sizeof g2380);
}

static void teardown(void)
{
    gfx_framebuffer_shutdown();
}

/* record: {attr, x_lo, strip_count, color} -- see asm_drawq.h. */
static void make_record(uint8_t *rec, dos_uchar attr, dos_uchar x_lo, dos_uchar strip_count, dos_uchar color)
{
    rec[0] = attr; rec[1] = x_lo; rec[2] = strip_count; rec[3] = color;
}

static void test_basic_three_part_bar(void)
{
    const char *t = "basic_three_part_bar";
    uint8_t list[1 + 4];
    setup();

    write_marker_bitmap(0x00, 0x11);            /* colour slot 0 left cap */
    write_marker_bitmap(0x82, 0x22);             /* colour slot 0 middle tile */
    write_marker_bitmap(0x82 + 0x62, 0x33);      /* colour slot 0 right cap */

    list[0] = 1; /* one record */
    make_record(&list[1], 5 /*attr*/, 20 /*x_lo -> y*/, 3 /*strip_count*/, 0 /*color -> slot 0*/);

    draw_queue_render_list(list, 0);

    check(t, count_marker_in_row(20, 0x11) == 2, "left cap must draw exactly one 1x1 bitmap (2 marker bytes)");
    check(t, count_marker_in_row(20, 0x22) == 6, "middle tile must be blitted strip_count(3) times (2 bytes each = 6)");
    check(t, count_marker_in_row(20, 0x33) == 2, "right cap must draw exactly one 1x1 bitmap (2 marker bytes)");
    check(t, count_marker_in_row(21, 0x11) == 0 && count_marker_in_row(19, 0x11) == 0,
          "non-highlighted render must not touch neighbouring rows (y is unscaled record.x_lo, not +0xB8)");

    teardown();
}

static void test_zero_strip_count_draws_only_caps(void)
{
    const char *t = "zero_strip_count_draws_only_caps";
    uint8_t list[1 + 4];
    setup();

    write_marker_bitmap(0x00, 0x44);
    write_marker_bitmap(0x82, 0x55);
    write_marker_bitmap(0x82 + 0x62, 0x66);

    list[0] = 1;
    make_record(&list[1], 1, 30, 0 /* strip_count == 0 */, 0);

    draw_queue_render_list(list, 0);

    check(t, count_marker_in_row(30, 0x44) == 2, "left cap must still draw with strip_count == 0");
    check(t, count_marker_in_row(30, 0x55) == 0, "middle tile must not draw at all when strip_count == 0");
    check(t, count_marker_in_row(30, 0x66) == 2, "right cap must still draw with strip_count == 0");

    teardown();
}

static void test_highlighted_shifts_y_and_restores_g96(void)
{
    const char *t = "highlighted_shifts_y_and_restores_g96";
    uint8_t list[1 + 4];
    setup();

    write_marker_bitmap(0x00, 0x77);
    write_marker_bitmap(0x82, 0x88);
    write_marker_bitmap(0x82 + 0x62, 0x99);

    list[0] = 1;
    make_record(&list[1], 2, 5 /* x_lo */, 1, 0);

    g96 = 0x1234 & 0x7fff; /* sentinel unequal to both 0x190 and 0x9f, to prove a literal restore, not save/restore */

    draw_queue_render_list(list, 1 /* highlighted */);

    /* y = x_lo(5) + 0xB8 = 0xBD = 189, NOT row 5. */
    check(t, count_marker_in_row(189, 0x77) == 2, "highlighted render must draw at y = record.x_lo + 0xB8");
    check(t, count_marker_in_row(5, 0x77) == 0, "highlighted render must NOT draw at the unshifted y");
    check(t, g96 == 0x9f, "highlighted render must restore g96 to the literal 0x9F on exit, not a saved prior value");

    teardown();
}

static void test_highlighted_pokes_and_restores_g2380_bytes(void)
{
    const char *t = "highlighted_pokes_and_restores_g2380_bytes";
    static const dos_uint offsets[3] = {0x021u, 0x0a3u, 0xb3fu}; /* first/second/last of the 24 */
    uint8_t list[1 + 4];
    setup();
    dos_char *flat = g2380_flat();

    /* Give each poked offset a recognisably wrong "before" value so a
     * missed restore would be caught. */
    flat[offsets[0]] = (dos_char)0x55;
    flat[offsets[1]] = (dos_char)0x55;
    flat[offsets[2]] = (dos_char)0x55;

    write_marker_bitmap(0x00, 0x01); /* record still needs a valid slot-0 bitmap to render without crashing */

    list[0] = 1;
    make_record(&list[1], 0, 0, 0, 0);

    draw_queue_render_list(list, 1 /* highlighted */);

    check(t, (dos_uchar)flat[offsets[0]] == 0x0b, "offset 0x021 must be restored to 0x0B after the highlighted pass");
    check(t, (dos_uchar)flat[offsets[1]] == 0x0b, "offset 0x0a3 must be restored to 0x0B after the highlighted pass");
    check(t, (dos_uchar)flat[offsets[2]] == 0x0b, "offset 0xb3f must be restored to 0x0B after the highlighted pass");

    teardown();
}

static void test_color_byte_cycles_across_frames(void)
{
    /* asm_drawq.h: the record's 4th byte (color) is rewritten every
     * render, +1 unless (color&3)==3, in which case -3 -- cycling
     * 4,5,6,7,4,5,6,7,... forever.  Verified here purely on the list
     * bytes, independent of gfx_* (no framebuffer needed). */
    const char *t = "color_byte_cycles_across_frames";
    uint8_t list[1 + 4];
    setup();

    /* The cycle 4,5,6,7,4,... only ever selects g2380 slots 4..7; those
     * are left zeroed by setup()'s memset (packed_bytes=rows=0), which
     * every render below treats as "draw nothing" -- safe, no seeding
     * needed for this test, which only checks the color byte itself. */
    list[0] = 1;
    make_record(&list[1], 0, 0, 0, 4); /* color starts at 4 (4 mod 4 == 0) */

    dos_uchar expect[5] = {5, 6, 7, 4, 5}; /* four renders' worth of resulting color byte */
    int i;
    for (i = 0; i < 5; i++) {
        draw_queue_render_list(list, 0);
        check(t, (dos_uchar)list[4] == expect[i], "record.color must cycle 4,5,6,7,4,... one step per render");
    }

    teardown();
}

static void test_multi_record_list(void)
{
    const char *t = "multi_record_list";
    uint8_t list[1 + 8];
    setup();

    write_marker_bitmap(0x00, 0xD1); /* slot 0 */
    write_marker_bitmap(0x82, 0xD2);
    write_marker_bitmap(0x82 + 0x62, 0xD3);

    list[0] = 2;
    make_record(&list[1], 0, 40, 0, 0); /* row 40 */
    make_record(&list[5], 0, 60, 0, 0); /* row 60 */

    draw_queue_render_list(list, 0);

    check(t, count_marker_in_row(40, 0xD1) == 2 && count_marker_in_row(40, 0xD3) == 2,
          "first record in a 2-record list must render");
    check(t, count_marker_in_row(60, 0xD1) == 2 && count_marker_in_row(60, 0xD3) == 2,
          "second record in a 2-record list must also render");

    teardown();
}

int main(void)
{
    test_basic_three_part_bar();
    test_zero_strip_count_draws_only_caps();
    test_highlighted_shifts_y_and_restores_g96();
    test_highlighted_pokes_and_restores_g2380_bytes();
    test_color_byte_cycles_across_frames();
    test_multi_record_list();

    if (g_failures) {
        fprintf(stderr, "test_asm_drawq: %d failure(s)\n", g_failures);
        return 1;
    }
    printf("test_asm_drawq: OK\n");
    return 0;
}
