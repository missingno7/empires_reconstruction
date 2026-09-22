/* test_asm_sprdraw.c -- unit tests for portable/game/asm_sprdraw.c
 * (sprite_table_queue_draws, animated_tile_tick, sprite_record_adjust_draw
 * -- the port of asm/SPRDRAW.ASM).
 *
 * `objtab` and `sprite_tile_bank` are generated BSS pointers (NULL by
 * default, per state-map.md they are the SAME objects as the ASM's own
 * `_g40d0`/`_sprbase` externs -- see asm_sprites.h); these tests point
 * them at small synthetic buffers they own.  draw_queue_append's effect
 * is observed directly through g2f30 (asm_drawqbuf.h), a real DGROUP
 * buffer.  animated_tile_tick's two-calls-sharing-one-argument-block
 * gfx_wipe_rect pattern is verified by checking exactly which framebuffer
 * bytes end up at the primary row vs. the 0xB8-mirror row, per
 * asm_sprdraw.c's own decode.
 */
#include "game.h"
#include "asm_sprites.h"
#include "asm_drawqbuf.h"

#include <stdio.h>
#include <string.h>

/* This test binary's link closure also includes asm_sprites.c (the two
 * ASM modules share portable/include/asm_sprites.h and are built/tested
 * together); it calls into board.c, which is not part of this minimal
 * closure -- see test_asm_sprites.c's identical stub for the rationale.
 * Not exercised by any test in this file. */

static int g_failures = 0;

static void fail(const char *test, const char *what)
{
    fprintf(stderr, "test_asm_sprdraw: FAIL %s: %s\n", test, what);
    g_failures++;
}

static void check(const char *test, int cond, const char *what)
{
    if (!cond) fail(test, what);
}

static void set_permissive_clip(void)
{
    g94 = -1; g96 = 500; g98 = -1; g9a = 500;
}

/* Frame bank: room for a few frames, each SPRITE_FRAME_STRIDE apart, with
 * a full 0x23-byte 1x1 VGA bitmap (16-byte colour table + header + one
 * packed data byte) at +SPRITE_FRAME_HEADER within each frame slot. */
static dos_char g_frame_bank[4 * SPRITE_FRAME_STRIDE + 0x23];

static void make_frame_bitmap(dos_uchar frame, dos_uchar marker)
{
    dos_char *base = g_frame_bank + (unsigned)frame * SPRITE_FRAME_STRIDE + SPRITE_FRAME_HEADER;
    /* gfx_copy_rect treats packed nibble VALUE 0 as transparent
     * unconditionally, regardless of table[0]'s content -- use nibble 1
     * (opaque) mapped to `marker` instead (see test_asm_sprites.c's
     * identical note). */
    memset(base, 0, 0x10);
    base[0x11] = (dos_char)marker; /* VGA colour table[1] */
    base[0x20] = 1;                /* 1 byte/row -> 2 pixel columns */
    base[0x21] = 1;                /* 1 row */
    base[0x22] = 0x10;              /* high nibble=1 (opaque, col X) low nibble=0 (transparent, col X+1) */
}

/* objtab/g40d0 buffer: count byte, then up to 8 3-byte records. */
static dos_uchar g_objtab_buf[1 + 8 * 3];

static void setup(void)
{
    display_mode = 5;
    gfx_framebuffer_init();
    memset(gfx_framebuffer(), 0, (size_t)gfx_row_bytes() * GFX_ROWS);
    set_permissive_clip();
    gbc = 0;
    result = 0x11;

    memset(g_frame_bank, 0, sizeof g_frame_bank);
    memset(g_objtab_buf, 0, sizeof g_objtab_buf);
    objtab = (dos_char *)g_objtab_buf;
    sprite_tile_bank = g_frame_bank;

    raycast_trail_active = 0;
    g0a20[0] = 0; g0a20[1] = 0;

    draw_queue_reset();
}

static void teardown(void)
{
    gfx_framebuffer_shutdown();
}

static void set_objtab_record(dos_uint i, dos_uchar x, dos_uchar y, dos_uchar flags)
{
    dos_uchar *rec = &g_objtab_buf[1 + i * 3];
    rec[0] = x; rec[1] = y; rec[2] = flags;
}

/* ---- sprite_table_queue_draws ------------------------------------------ */

static void test_queue_draws_appends_records_and_blits(void)
{
    const char *t = "queue_draws_appends_records_and_blits";
    setup();

    g_objtab_buf[0] = 2;
    set_objtab_record(0, 10, 20, 0); /* frame 0 */
    set_objtab_record(1, 15, 25, 1); /* frame 1 */
    make_frame_bitmap(0, 0xAA);
    make_frame_bitmap(1, 0xBB);

    sprite_table_queue_draws();

    check(t, g2f30[0] == 2, "g2f30 record count == 2");
    check(t, g2f30[1] == 0x30 && g2f30[2] == 10 && g2f30[3] == 20 && g2f30[4] == 0x0F && g2f30[5] == 0x1E,
          "record 0: attr=0x30 (first slot), x=10, y=20, color=0x0F, height=0x1E");
    check(t, g2f30[6] == 0x31 && g2f30[7] == 15 && g2f30[8] == 25 && g2f30[9] == 0x0F && g2f30[10] == 0x1E,
          "record 1: attr=0x31 (slot incremented), x=15, y=25, color=0x0F, height=0x1E");

    /* gfx_copy_rect(x*2, y+0xB8, frame_ptr, 0): record0 -> (20, 204); record1 -> (30, 209). */
    check(t, g3924[20 + 0xB8][20] == 0xAA, "record 0's frame blitted at (x*2, y+0xB8)");
    check(t, g3924[25 + 0xB8][30] == 0xBB, "record 1's frame blitted at (x*2, y+0xB8)");

    {
        dos_uint val = (dos_uint)(g0a20[0] | (g0a20[1] << 8));
        check(t, val == 10, "g0a20 (==ga20) set to 10 once objtab is exhausted");
    }

    teardown();
}

/* ---- animated_tile_tick ------------------------------------------------- */

static void test_animated_tile_tick_throttle_and_redraw_sequence(void)
{
    const char *t = "animated_tile_tick_throttle_and_redraw_sequence";
    setup();

    g_objtab_buf[0] = 2;
    set_objtab_record(0, 5, 30, (dos_uchar)(SPRITE_ANIM_FLAG_ARMED | 3)); /* armed, frame 3, dir=up */
    set_objtab_record(1, 40, 60, 3); /* NOT armed -- must be skipped entirely */
    make_frame_bitmap(4, 0xCC); /* frame 3 advances to 4 (dir=up) */

    /* Source rows for the wipe (y+0x148): row 30+0x148=358, row 60+0x148=388. */
    memset(g3924[358], 0xEE, GFX_ROW_BYTES_VGA);
    memset(g3924[388], 0xFF, GFX_ROW_BYTES_VGA);

    g0a20[0] = 3; g0a20[1] = 0; /* two calls before this one already happened */

    animated_tile_tick(); /* counter 3->2: no tick */
    {
        dos_uint val = (dos_uint)(g0a20[0] | (g0a20[1] << 8));
        check(t, val == 2, "counter decrements when not yet due");
    }
    check(t, g3924[30][10] == 0, "no redraw happens before the counter reaches 0");

    animated_tile_tick(); /* 2->1 */
    animated_tile_tick(); /* 1->0: tick runs now */

    {
        dos_uint val = (dos_uint)(g0a20[0] | (g0a20[1] << 8));
        check(t, val == 10, "counter reset to 10 once the tick actually runs");
    }

    /* record 0: x*2=10, y=30. Expect (per asm_sprdraw.c's decode):
     *   row 30:  col10 = 0xCC (gfx_copy_rect marker), cols 11..39 = 0xEE (wiped from row 358)
     *   row 214 (30+0xB8): identical to row 30's POST-copy_rect content
     *                       (col10=0xCC, cols 11..39=0xEE), because the
     *                       4th call wipes row 30 into row 214 AFTER the
     *                       marker was drawn. */
    check(t, g3924[30][10] == 0xCC, "record 0 frame marker drawn at (x*2, y)");
    check(t, g3924[30][11] == 0xEE && g3924[30][39] == 0xEE, "record 0 primary row background restored from the y+0x148 source");
    check(t, g3924[214][10] == 0xCC, "record 0 mirror row picks up the freshly-drawn marker via the 4th wipe");
    check(t, g3924[214][11] == 0xEE && g3924[214][39] == 0xEE, "record 0 mirror row background matches the primary row");

    /* record 1 (not armed): completely untouched. */
    check(t, g3924[60][80] == 0, "unarmed record must not be redrawn");
    check(t, g3924[60 + 0xB8][80] == 0, "unarmed record's mirror row must not be touched either");

    {
        dos_uchar flags = g_objtab_buf[1 + 1 * 3 + 2];
        check(t, flags == 3, "unarmed record's flags byte (frame/direction) must not advance");
    }
    {
        dos_uchar flags = g_objtab_buf[1 + 0 * 3 + 2];
        check(t, (flags & SPRITE_ANIM_FRAME_MASK) == 4, "armed record's frame advances 3 -> 4 (dir=up)");
        check(t, (flags & SPRITE_ANIM_FLAG_ARMED) != 0, "armed bit is preserved across the advance");
    }

    teardown();
}

static void test_animated_tile_tick_skips_when_raycast_trail_active(void)
{
    const char *t = "animated_tile_tick_skips_when_raycast_trail_active";
    setup();
    raycast_trail_active = 1; /* per the literal polarity finding: nonzero -> skip entirely */

    g_objtab_buf[0] = 1;
    set_objtab_record(0, 5, 30, (dos_uchar)(SPRITE_ANIM_FLAG_ARMED | 3));
    g0a20[0] = 1; g0a20[1] = 0; /* would tick on this very call if not gated */

    animated_tile_tick();

    dos_uint val = (dos_uint)(g0a20[0] | (g0a20[1] << 8));
    check(t, val == 1, "raycast_trail_active != 0 must skip the routine entirely, including the counter decrement");
    check(t, g3924[30][10] == 0, "no redraw when gated off");

    teardown();
}

/* ---- sprite_record_adjust_draw ------------------------------------------ */

static void test_sprite_record_adjust_draw_advances_and_redraws(void)
{
    const char *t = "sprite_record_adjust_draw_advances_and_redraws";
    setup();

    g_objtab_buf[0] = 2;
    set_objtab_record(0, 1, 2, 0); /* id 0: unrelated, untouched */
    set_objtab_record(1, 8, 12, (dos_uchar)(SPRITE_ANIM_FLAG_DIR_DOWN | 0)); /* id 1: frame=0, dir=down -> wraps to 0x17 */
    make_frame_bitmap(0x17, 0xDD);

    memset(g3924[12 + 0x148], 0x33, GFX_ROW_BYTES_VGA);

    sprite_record_adjust_draw(1); /* NOTE: no "armed" bit test in this routine, unlike animated_tile_tick */

    {
        dos_uchar flags = g_objtab_buf[1 + 1 * 3 + 2];
        check(t, (flags & SPRITE_ANIM_FRAME_MASK) == SPRITE_ANIM_FRAME_MAX, "frame 0 wraps down to 0x17 (not 0x1F)");
    }
    check(t, g3924[12][16] == 0xDD, "redraw at (x*2=16, y=12) uses the advanced frame's bitmap");
    check(t, g3924[12 + 0xB8][16] == 0xDD, "mirror row picks up the freshly-drawn marker");

    {
        dos_uchar flags0 = g_objtab_buf[1 + 0 * 3 + 2];
        check(t, flags0 == 0, "record id 0 (not selected) must be untouched");
    }

    teardown();
}

int main(void)
{
    test_queue_draws_appends_records_and_blits();
    test_animated_tile_tick_throttle_and_redraw_sequence();
    test_animated_tile_tick_skips_when_raycast_trail_active();
    test_sprite_record_adjust_draw_advances_and_redraws();

    if (g_failures) {
        fprintf(stderr, "test_asm_sprdraw: %d failure(s)\n", g_failures);
        return 1;
    }
    printf("test_asm_sprdraw: OK\n");
    return 0;
}
