/* test_asm_sprites.c -- unit tests for portable/game/asm_sprites.c
 * (play_window_wipe_clipped, sprite_table_wipe_active, board_actors_draw,
 * sprite_script_frame_driver -- the port of asm/SPRITES.ASM).
 *
 * Uses the same "marker byte" 1x1 VGA bitmap technique as
 * test_asm_iconanim.c/test_asm_drawq.c, and the same real-framebuffer
 * observation style as test_asm_rectq.c/test_gfx.c, against a real
 * gfx_framebuffer_init(display_mode=5) framebuffer.
 *
 * Synthetic bytecode programs for sprite_script_frame_driver() are placed
 * inside a high sub-offset of actor_sprite_dims_table (a real dos_uchar[672]
 * byte array) and addressed relative to actor_record_table, exactly like
 * the historical DS:B3AE-relative encoding (see asm_sprites.h's
 * SPRITE_SCRIPT_BASE note). This crosses two separate extern objects
 * (actor_record_table, actor_sprite_dims_table) with no standard-C
 * guarantee about their relative placement or even which one comes first
 * -- place_bytecode() below verifies the resulting offset is a small
 * positive value that fits a 16-bit word before trusting it, and SKIPs
 * (exit 77) if it does not (see place_bytecode's own comment for why
 * actor_state_table itself could not be used for this in this build).
 */
#include "game.h"
#include "asm_sprites.h"

#include <stdio.h>
#include <string.h>
#include <stddef.h>

/* record_field_skip_n/board_run_unit_script/board_advance_unit_moves are
 * declared in game_funcs.h (src/BOARD.C's port, portable/game/board.c) but
 * board.c is not part of this test's link closure (it pulls in a large,
 * still-incomplete UI/menu subsystem unrelated to sprite scripts) --
 * stubbed here per the TU brief's rule for not-yet-linkable callees. Only
 * OP_DISPATCH_2A2D/OP_DISPATCH_36F0 in sprite_script_frame_driver call
 * these; no test below exercises those two opcodes. */
static int g_stub_board_run_unit_script_calls = 0;
static int g_stub_board_advance_unit_moves_calls = 0;

dos_char *record_field_skip_n(dos_int n)
{
    (void)n;
    return NULL;
}

void board_run_unit_script(dos_uchar *s)
{
    (void)s;
    g_stub_board_run_unit_script_calls++;
}

void board_advance_unit_moves(dos_int a)
{
    (void)a;
    g_stub_board_advance_unit_moves_calls++;
}

static int g_failures = 0;

static void fail(const char *test, const char *what)
{
    fprintf(stderr, "test_asm_sprites: FAIL %s: %s\n", test, what);
    g_failures++;
}

static void check(const char *test, int cond, const char *what)
{
    if (!cond) fail(test, what);
}

/* One 0x23-byte minimal VGA bitmap (16-byte colour table + 1x1 header +
 * one packed data byte), same shape test_asm_iconanim.c uses. */
static dos_char g_bitmaps[8][0x23];

static dos_char *make_bitmap(int slot, dos_uchar marker)
{
    /* gfx_copy_rect treats packed nibble VALUE 0 as transparent
     * unconditionally (test_gfx.c's own test_copy_rect_transparency),
     * regardless of what colour table[0] holds -- use nibble 1 (opaque)
     * mapped to `marker` instead. */
    memset(g_bitmaps[slot], 0, 0x10);
    g_bitmaps[slot][0x11] = (dos_char)marker; /* VGA colour table[1] */
    g_bitmaps[slot][0x20] = 1;                /* 1 byte/row -> 2 pixel columns */
    g_bitmaps[slot][0x21] = 1;                /* 1 row */
    g_bitmaps[slot][0x22] = 0x10;              /* high nibble=1 (opaque, col X) low nibble=0 (transparent, col X+1) */
    return g_bitmaps[slot];
}

static void set_permissive_clip(void)
{
    g94 = -1; g96 = 500; g98 = -1; g9a = 500;
}

static void setup(void)
{
    display_mode = 5;
    gfx_framebuffer_init();
    memset(gfx_framebuffer(), 0, (size_t)gfx_row_bytes() * GFX_ROWS);
    set_permissive_clip();
    gbc = 0;

    actor_record_table[0] = 0;
    memset(actor_state_table, 0, sizeof(actor_state_table[0]) * 12);
    memset(tile_height_table, 0, sizeof(tile_height_table[0]) * 84);
    memset(tile_width_table, 0, sizeof(tile_width_table[0]) * 84);
    memset(resource_ptr_table, 0, sizeof(resource_ptr_table[0]) * 84);
    memset(actor_sprite_dims_table, 0, sizeof(actor_sprite_dims_table[0]) * 672);
    b4374[0] = 0;
    board_record_index = 0;
    raycast_trail_active = 0;
    hud_scroll_cooldown_ticks = 0;
    g72e = 0; cursor_facing_left = 0; cursor_x = 0; cursor_y = 0;
    g40ce = 0; gc04e = 0;
    memset(b740, 0, sizeof(b740[0]) * 278);
}

static void teardown(void)
{
    gfx_framebuffer_shutdown();
}

/* ---- play_window_wipe_clipped ---------------------------------------- */

static void test_pwwc_accept_copies_from_mirror_row(void)
{
    const char *t = "pwwc_accept_copies_from_mirror_row";
    setup();

    memset(g3924[40 + 0xB8], 0xAB, GFX_ROW_BYTES_VGA);
    memset(g3924[41 + 0xB8], 0xAB, GFX_ROW_BYTES_VGA);
    memset(g3924[40], 0, GFX_ROW_BYTES_VGA);
    memset(g3924[41], 0, GFX_ROW_BYTES_VGA);

    play_window_wipe_clipped(20, 40, 6, 2); /* fully inside the play window */

    check(t, g3924[40][20] == 0xAB && g3924[40][25] == 0xAB, "row 40 span copied from the 0xB8 mirror row");
    check(t, g3924[41][20] == 0xAB && g3924[41][25] == 0xAB, "row 41 span copied from the 0xB8 mirror row");
    check(t, g3924[40][19] == 0 && g3924[40][26] == 0, "columns outside the rect stay untouched");

    teardown();
}

static void test_pwwc_rejects_outside_window(void)
{
    const char *t = "pwwc_rejects_outside_window";
    setup();

    memset(g3924[40 + 0xB8], 0xAB, GFX_ROW_BYTES_VGA);
    memset(g3924[40], 0x00, GFX_ROW_BYTES_VGA);

    play_window_wipe_clipped(0x138, 40, 6, 2); /* x1 > 0x137 */
    check(t, g3924[40][20] == 0, "x1 > 0x137 must reject without calling gfx_wipe_rect");

    play_window_wipe_clipped(20, 0xA0, 6, 2); /* y1 > 0x9F */
    check(t, g3924[0xA0][20] == 0, "y1 > 0x9F must reject");

    play_window_wipe_clipped(1, 40, 2, 2); /* x2 = 1+2-1 = 2 < 8 */
    check(t, g3924[40][1] == 0, "x1+w-1 < 8 must reject");

    play_window_wipe_clipped(20, 1, 2, 2); /* y2 = 1+2-1 = 2 < 0x10 */
    check(t, g3924[1][20] == 0, "y1+h-1 < 0x10 must reject");

    teardown();
}

/* ---- sprite_table_wipe_active ------------------------------------------ */

static void test_sprite_table_wipe_active_filters_by_board_and_active(void)
{
    const char *t = "sprite_table_wipe_active_filters_by_board_and_active";
    setup();
    board_record_index = 3;

    memset(g3924[60 + 0xB8], 0xCD, GFX_ROW_BYTES_VGA);
    memset(g3924[60], 0, GFX_ROW_BYTES_VGA);
    memset(g3924[80 + 0xB8], 0xCD, GFX_ROW_BYTES_VGA);
    memset(g3924[80], 0, GFX_ROW_BYTES_VGA);
    memset(g3924[100 + 0xB8], 0xCD, GFX_ROW_BYTES_VGA);
    memset(g3924[100], 0, GFX_ROW_BYTES_VGA);

    tile_width_table[0] = 6;
    tile_height_table[0] = 2;

    actor_record_table[0] = 3;
    {
        struct actor_record *r0 = (struct actor_record *)&actor_state_table[0];
        struct actor_record *r1 = (struct actor_record *)&actor_state_table[1];
        struct actor_record *r2 = (struct actor_record *)&actor_state_table[2];
        r0->board_id = 3; r0->active = 0; r0->x = 20; r0->y = 60; r0->sprite_frame = 0;
        r1->board_id = 9; r1->active = 0; r1->x = 20; r1->y = 80; r1->sprite_frame = 0; /* wrong board */
        r2->board_id = 3; r2->active = 1; r2->x = 20; r2->y = 100; r2->sprite_frame = 0; /* active==1 -> skipped */
    }

    sprite_table_wipe_active();

    check(t, g3924[60][20] == 0xCD, "matching record's row got wiped");
    check(t, g3924[80][20] == 0, "wrong-board record must not be wiped");
    check(t, g3924[100][20] == 0, "active==1 record must not be wiped");

    teardown();
}

/* ---- board_actors_draw -------------------------------------------------- */

static void test_board_actors_draw_blits_and_vline(void)
{
    const char *t = "board_actors_draw_blits_and_vline";
    setup();
    board_record_index = 5;
    /* board_actors_draw() itself calls gfx_color_select(0x0F) first, which
     * overwrites `result` from g3904[0x0F] (mode 5) -- seed THAT instead
     * of setting `result` directly. */
    g3904[0x0F] = 0x77;

    resource_ptr_table[2] = make_bitmap(0, 0x42);

    actor_record_table[0] = 1;
    {
        struct actor_record *r0 = (struct actor_record *)&actor_state_table[0];
        r0->board_id = 5; r0->active = 0; r0->x = 30; r0->y = 15; r0->sprite_frame = 2; r0->dir_flip = 0;
        r0->vline_extra = 3;
    }

    board_actors_draw(10); /* y0 = 10 -> blit at (30, 25) */

    check(t, g3924[25][30] == 0x42, "sprite frame marker byte blitted at (x, y0+y)");

    /* gfx_vline args, transcribed literally from the ASM (see asm_sprites.c's
     * note): x=rec.x+0x10=46, y=vline_extra=3, n=rec.y-vline_extra+1=13 ->
     * a vertical strip at column 46, rows 3..15. */
    {
        int row, hit = 1;
        for (row = 3; row <= 15; row++) {
            if (g3924[row][46] != 0x77) { hit = 0; break; }
        }
        check(t, hit, "gfx_vline strip drawn exactly where the literal (swapped) argument mapping predicts");
    }

    teardown();
}

/* ---- sprite_script_frame_driver ----------------------------------------- */

/* Scratch bytecode storage: actor_state_table turned out to sit BEHIND
 * actor_record_table in this build's memory layout (a negative, therefore
 * unrepresentable, SPRITE_SCRIPT_BASE-relative offset -- see
 * asm_sprites.h's SPRITE_SCRIPT_BASE note; the historical 16-bit
 * wraparound this offset relies on has no equivalent on a flat address
 * space). actor_sprite_dims_table (a genuine dos_uchar[672] byte array,
 * not a reinterpreted pointer array) empirically sits FORWARD of
 * actor_record_table instead, so a suboffset within it is used as scratch
 * program storage; sub_offset 500+ stays well clear of the small
 * collision-dims indices (< ~60) any test record's sprite_frame/dir_flip
 * can compute into this same array. */
static dos_uint place_bytecode(dos_uint sub_offset, const dos_uchar *bytes, size_t n)
{
    dos_uchar *dst = &actor_sprite_dims_table[sub_offset];
    ptrdiff_t diff;
    memcpy(dst, bytes, n);
    diff = dst - (dos_uchar *)actor_record_table;
    if (diff < 0 || diff > 0xFFFF) {
        fprintf(stderr,
            "test_asm_sprites: actor_record_table/actor_sprite_dims_table are not "
            "reachable via a positive 16-bit SPRITE_SCRIPT_BASE-relative offset in "
            "this build (diff=%td) -- see asm_sprites.h's SPRITE_SCRIPT_BASE note. "
            "Skipping.\n", diff);
        exit(77);
    }
    return (dos_uint)diff;
}

static void test_fast_path_render_and_active_gate(void)
{
    const char *t = "fast_path_render_and_active_gate";
    setup();
    board_record_index = 1;
    resource_ptr_table[4] = make_bitmap(1, 0x55);

    actor_record_table[0] = 2;
    {
        struct actor_record *r0 = (struct actor_record *)&actor_state_table[0];
        struct actor_record *r1 = (struct actor_record *)&actor_state_table[1];
        r0->board_id = 1; r0->rendered = 1; r0->active = 0; r0->x = 50; r0->y = 60; r0->sprite_frame = 4;
        r1->board_id = 1; r1->rendered = 1; r1->active = 1; r1->x = 70; r1->y = 60; r1->sprite_frame = 4; /* active==1: must NOT draw */
    }

    sprite_script_frame_driver();

    check(t, g3924[60][50] == 0x55, "rendered==1 fast path blits directly, bypassing the bytecode program");
    check(t, g3924[60][70] == 0, "active==1 must suppress the draw even on the rendered==1 fast path");

    teardown();
}

static void test_countdown_decrements_without_running_program(void)
{
    const char *t = "countdown_decrements_without_running_program";
    setup();
    board_record_index = 1;

    actor_record_table[0] = 1;
    {
        struct actor_record *r0 = (struct actor_record *)&actor_state_table[0];
        r0->board_id = 1; r0->rendered = 0; r0->countdown = 3;
        r0->saved_pc = 0xFFFF; /* poison: if this were dereferenced it reads far out of any table */
    }

    sprite_script_frame_driver();
    check(t, ((struct actor_record *)&actor_state_table[0])->countdown == 2, "countdown decrements by exactly one per call");

    sprite_script_frame_driver();
    check(t, ((struct actor_record *)&actor_state_table[0])->countdown == 1, "countdown keeps decrementing");

    teardown();
}

static void test_program_set_velocity_then_move_clamped(void)
{
    const char *t = "program_set_velocity_then_move_clamped";
    setup();
    board_record_index = 1;
    resource_ptr_table[7] = make_bitmap(2, 0x91);

    {
        const dos_uchar prog[] = {
            OP_SET_LIMITS, 0, 20,      /* frame_limit_lo=0, frame_limit_hi=20 */
            OP_SET_VELOCITY, 5,        /* sprite_frame=5, dir_flip=0 */
            OP_MOVE_CLAMPED, 3, 2, 2,  /* x+=3, y+=2, frame = clamp(5+2)=7, dir_flip=0 */
        };
        dos_uint pc = place_bytecode(500, prog, sizeof prog);

        actor_record_table[0] = 1;
        {
            struct actor_record *r0 = (struct actor_record *)&actor_state_table[0];
            r0->board_id = 1; r0->rendered = 0; r0->countdown = 0;
            r0->x = 40; r0->y = 50;
            r0->saved_pc = pc;
        }
    }

    sprite_script_frame_driver();

    {
        struct actor_record *r0 = (struct actor_record *)&actor_state_table[0];
        check(t, r0->sprite_frame == 7, "sprite_frame == 5 (SET_VELOCITY) + 2 (MOVE_CLAMPED delta), within [0,20]");
        check(t, r0->x == 43 && r0->y == 52, "x/y advanced by the MOVE_CLAMPED signed delta");
    }
    /* gfx_copy_rect's VGA driver rounds x down to an even packed-pair
     * column (bxcol = x>>1, dest column = 2*bxcol) before drawing -- x=43
     * (odd) therefore lands at column 42, not 43 (portable/gfx/gfx_vga.c
     * vga_copy_rect); unrelated to this module's own port, just gfx_copy_
     * rect's own packed-pixel addressing. */
    check(t, g3924[52][42] == 0x91, "MOVE_CLAMPED redraws at the new position using resource_ptr_table[new frame]");

    teardown();
}

static void test_program_add_offset_skips_poison(void)
{
    const char *t = "program_add_offset_skips_poison";
    setup();
    board_record_index = 1;
    resource_ptr_table[9] = make_bitmap(3, 0xEE); /* MOVE_CLAMPED redraws using frame 9's bitmap */

    {
        const dos_uchar prog[] = {
            OP_ADD_OFFSET, 2, 0,     /* si (=3 after this operand) += 2 -> lands on index 5 */
            OP_SET_FLAG1, 1,         /* POISON: if reached, sets actor_state_table[1].rendered = 1 */
            OP_SET_LIMITS, 0, 20,    /* index 5: real program resumes here */
            OP_SET_VELOCITY, 9,
            OP_MOVE_CLAMPED, 0, 0, 0,
        };
        dos_uint pc = place_bytecode(500, prog, sizeof prog);

        actor_record_table[0] = 1; /* record 1 is never iterated by the outer loop -- pure poison target */
        {
            struct actor_record *r0 = (struct actor_record *)&actor_state_table[0];
            struct actor_record *r1 = (struct actor_record *)&actor_state_table[1];
            r0->board_id = 1; r0->rendered = 0; r0->countdown = 0; r0->x = 5; r0->y = 5;
            r0->saved_pc = pc;
            r1->rendered = 0;
        }
    }

    sprite_script_frame_driver();

    {
        struct actor_record *r0 = (struct actor_record *)&actor_state_table[0];
        struct actor_record *r1 = (struct actor_record *)&actor_state_table[1];
        check(t, r1->rendered == 0, "OP_ADD_OFFSET must skip the poison OP_SET_FLAG1 entirely");
        check(t, r0->sprite_frame == 9, "execution resumes exactly at the jump target");
    }

    teardown();
}

int main(void)
{
    test_pwwc_accept_copies_from_mirror_row();
    test_pwwc_rejects_outside_window();
    test_sprite_table_wipe_active_filters_by_board_and_active();
    test_board_actors_draw_blits_and_vline();
    test_fast_path_render_and_active_gate();
    test_countdown_decrements_without_running_program();
    test_program_set_velocity_then_move_clamped();
    test_program_add_offset_skips_poison();

    if (g_failures) {
        fprintf(stderr, "test_asm_sprites: %d failure(s)\n", g_failures);
        return 1;
    }
    printf("test_asm_sprites: OK\n");
    return 0;
}
