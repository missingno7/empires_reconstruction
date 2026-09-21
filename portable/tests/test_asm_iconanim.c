/* test_asm_iconanim.c -- unit tests for portable/game/asm_iconanim.c
 * (icon_list_animate_draw / icon_frame_reset_and_draw, the port of
 * asm/ICONANIM.ASM).
 *
 * Exercises icon_list_animate_draw() against a real
 * gfx_framebuffer_init(display_mode=5) framebuffer, a synthetic
 * icon_record_list_ptr list (fixed 12-byte records -- see asm_iconanim.h
 * for why this port corrects the inventory's "variable-length" read),
 * and a synthetic a72b2[] table of minimal 1x1 VGA bitmaps (same marker-
 * byte technique as test_asm_drawq.c, so this test can count markers in a
 * framebuffer row instead of re-deriving gfx_blit_bitmap's own pixel
 * arithmetic).
 */
#include "game.h"
#include "asm_iconanim.h"

#include <stdio.h>
#include <string.h>

static int g_failures = 0;

static void fail(const char *test, const char *what)
{
    fprintf(stderr, "test_asm_iconanim: FAIL %s: %s\n", test, what);
    g_failures++;
}

static void check(const char *test, int cond, const char *what)
{
    if (!cond) fail(test, what);
}

/* One 0x23-byte minimal VGA bitmap per a72b2 slot used by these tests:
 * 16-byte colour table + (packed_bytes=1, rows=1) header + one 0x00
 * packed data byte, so both destination pixels come out as table[0]. */
static dos_char g_bitmaps[8][0x23];

static void make_bitmap(int slot, dos_uchar marker)
{
    memset(g_bitmaps[slot], 0, 0x10);
    g_bitmaps[slot][0x10] = (dos_char)marker;
    g_bitmaps[slot][0x20] = 1;
    g_bitmaps[slot][0x21] = 1;
    g_bitmaps[slot][0x22] = 0;
    a72b2[slot] = g_bitmaps[slot];
}

static void set_permissive_clip(void)
{
    g94 = -1; g96 = 500; g98 = -1; g9a = 500;
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
    gbc = 0;
    memset(a72b2, 0, sizeof a72b2);
}

static void teardown(void)
{
    gfx_framebuffer_shutdown();
}

/* rec: cursor, xy_lo, xy_hi, frames[9] (asm_iconanim.h). */
static void make_record(uint8_t *rec, dos_uchar cursor, dos_uchar xy_lo, dos_uchar xy_hi,
                         const dos_uchar frames[9])
{
    rec[0] = cursor;
    rec[1] = xy_lo;
    rec[2] = xy_hi;
    memcpy(&rec[3], frames, 9);
}

static void test_advance_path_draws_correct_frame_and_position(void)
{
    const char *t = "advance_path_draws_correct_frame_and_position";
    uint8_t list[1 + 12];
    static const dos_uchar frames[9] = {5, 0, 0, 0, 0, 0, 0, 0, 0};
    setup();
    make_bitmap(4, 0x22); /* frames[0]=5 -> index = 5-1 = 4 */

    list[0] = 1;
    make_record(&list[1], 0, 10 /* xy_lo */, 30 /* xy_hi */, frames);
    icon_record_list_ptr = &list[0];

    icon_list_animate_draw();

    check(t, (dos_uchar)list[1] == 1, "cursor must be incremented to 1 after the advance path");
    check(t, count_marker_in_row(30, 0x22) == 2, "must draw a72b2[frames[0]-1] at y = xy_hi (unscaled)");
    check(t, count_marker_in_row(29, 0x22) == 0 && count_marker_in_row(31, 0x22) == 0,
          "must not draw into neighbouring rows");

    teardown();
}

static void test_cursor_walks_seeded_frame_sequence(void)
{
    const char *t = "cursor_walks_seeded_frame_sequence";
    uint8_t list[1 + 12];
    /* frames[0..2] chosen so (frame-1) stays in {0,1} -- both seeded. */
    static const dos_uchar frames[9] = {1, 2, 1, 0, 0, 0, 0, 0, 0};
    setup();
    make_bitmap(0, 0x41); /* index 0 (frame value 1) */
    make_bitmap(1, 0x42); /* index 1 (frame value 2) */

    list[0] = 1;
    make_record(&list[1], 0, 0, 60, frames);
    icon_record_list_ptr = &list[0];

    icon_list_animate_draw(); /* cursor 0: frames[0]=1 -> index 0 */
    check(t, count_marker_in_row(60, 0x41) == 2, "cursor 0 step must draw index 0 (frame value 1)");
    check(t, (dos_uchar)list[1] == 1, "cursor advances 0 -> 1");

    memset(gfx_framebuffer(), 0, (size_t)gfx_row_bytes() * GFX_ROWS);
    icon_list_animate_draw(); /* cursor 1: frames[1]=2 -> index 1 */
    check(t, count_marker_in_row(60, 0x42) == 2, "cursor 1 step must draw index 1 (frame value 2)");
    check(t, (dos_uchar)list[1] == 2, "cursor advances 1 -> 2");

    memset(gfx_framebuffer(), 0, (size_t)gfx_row_bytes() * GFX_ROWS);
    icon_list_animate_draw(); /* cursor 2: frames[2]=1 -> index 0 again */
    check(t, count_marker_in_row(60, 0x41) == 2, "cursor 2 step must draw index 0 again (frame value 1)");
    check(t, (dos_uchar)list[1] == 3, "cursor advances 2 -> 3");

    teardown();
}

static void test_reset_path(void)
{
    const char *t = "reset_path";
    uint8_t list[1 + 12];
    /* cursor starts at 4; frames[4] == 0 triggers the reset path:
     * cursor is SET to 1 (not incremented), and this call draws
     * a72b2[frames[0]] RAW (no -1 adjustment) -- asm_iconanim.h. */
    static const dos_uchar frames[9] = {3, 0, 0, 0, 0, 0, 0, 0, 0};
    setup();
    make_bitmap(3, 0x51); /* frames[0] == 3 used directly as the index (no -1) */

    list[0] = 1;
    make_record(&list[1], 4 /* cursor */, 0, 70, frames);
    icon_record_list_ptr = &list[0];

    icon_list_animate_draw();

    check(t, count_marker_in_row(70, 0x51) == 2, "reset path must draw a72b2[frames[0]] raw, no -1 adjustment");
    check(t, (dos_uchar)list[1] == 1, "reset path must SET cursor to 1, not increment it from 4");

    teardown();
}

static void test_multi_record_list(void)
{
    const char *t = "multi_record_list";
    uint8_t list[1 + 24];
    static const dos_uchar frames_a[9] = {1, 0, 0, 0, 0, 0, 0, 0, 0};
    static const dos_uchar frames_b[9] = {1, 0, 0, 0, 0, 0, 0, 0, 0};
    setup();
    make_bitmap(0, 0x61);

    list[0] = 2;
    make_record(&list[1], 0, 0, 80, frames_a);
    make_record(&list[13], 0, 0, 90, frames_b);
    icon_record_list_ptr = &list[0];

    icon_list_animate_draw();

    check(t, count_marker_in_row(80, 0x61) == 2, "first of two records must draw");
    check(t, count_marker_in_row(90, 0x61) == 2, "second of two records (at record_start+12) must draw");
    check(t, (dos_uchar)list[1] == 1 && (dos_uchar)list[13] == 1, "both records' cursors must advance independently");

    teardown();
}

static void test_x_is_doubled(void)
{
    const char *t = "x_is_doubled";
    uint8_t list[1 + 12];
    static const dos_uchar frames[9] = {1, 0, 0, 0, 0, 0, 0, 0, 0};
    setup();
    make_bitmap(0, 0x71);

    list[0] = 1;
    make_record(&list[1], 0, 7 /* xy_lo: x should render at column 14, not 7 */, 100, frames);
    icon_record_list_ptr = &list[0];

    icon_list_animate_draw();

    check(t, g3924[100][14] == 0x71 && g3924[100][15] == 0x71,
          "x must be xy_lo*2 (record xy_lo=7 -> columns 14,15), matching the ASM's `shl cx,1`");

    teardown();
}

static void test_icon_frame_reset_and_draw_is_a_documented_noop(void)
{
    const char *t = "icon_frame_reset_and_draw_is_a_documented_noop";
    /* No real C caller exists (asm_iconanim.h); just confirm it exists,
     * links, and doesn't touch global state. */
    dos_int before_g94 = g94;
    icon_frame_reset_and_draw();
    check(t, g94 == before_g94, "icon_frame_reset_and_draw() must be a no-op (no reachable standalone semantics -- see asm_iconanim.h)");
}

int main(void)
{
    test_advance_path_draws_correct_frame_and_position();
    test_cursor_walks_seeded_frame_sequence();
    test_reset_path();
    test_multi_record_list();
    test_x_is_doubled();
    test_icon_frame_reset_and_draw_is_a_documented_noop();

    if (g_failures) {
        fprintf(stderr, "test_asm_iconanim: %d failure(s)\n", g_failures);
        return 1;
    }
    printf("test_asm_iconanim: OK\n");
    return 0;
}
