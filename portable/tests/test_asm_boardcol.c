/* test_asm_boardcol.c -- unit tests for portable/game/asm_boardcol.c
 * (board_collision_span_or, the port of asm/BOARDCOL.ASM's
 * _board_collision_span_or).
 *
 * board_records is a generated BSS pointer (NULL by default); these tests
 * point it at a small synthetic grid they own, matching
 * BOARD_COLLISION_ROW_STRIDE (asm_boardcol.h).
 */
#include "game.h"
#include "asm_boardcol.h"

#include <stdio.h>
#include <string.h>

#define GRID_ROWS 40
#define GRID_BYTES (GRID_ROWS * BOARD_COLLISION_ROW_STRIDE)

static dos_char g_grid[GRID_BYTES];

static int g_failures = 0;

static void fail(const char *test, const char *what)
{
    fprintf(stderr, "test_asm_boardcol: FAIL %s: %s\n", test, what);
    g_failures++;
}

static void check(const char *test, int cond, const char *what)
{
    if (!cond) fail(test, what);
}

static void reset_grid(void)
{
    memset(g_grid, 0, sizeof g_grid);
    board_records = g_grid;
}

/* Independently reproduces the ASM's own offset formula (not by calling
 * into asm_boardcol.c) so the test has a from-scratch oracle for which
 * grid byte a given (x,y) selects, per asm/BOARDCOL.ASM / asm_boardcol.h:
 *   col = clamp(x,8,0x137)/8 - 1;  row = clamp(y,0x10,0x9F)/8 - 2;
 *   offset = row*0x26 + col. */
static int span_offset(dos_int x, dos_int y)
{
    dos_int cx = x, cy = y;
    if (cx > BOARD_COLLISION_X_MAX) cx = BOARD_COLLISION_X_MAX;
    if (cx < BOARD_COLLISION_X_MIN) cx = BOARD_COLLISION_X_MIN;
    if (cy > BOARD_COLLISION_Y_MAX) cy = BOARD_COLLISION_Y_MAX;
    if (cy < BOARD_COLLISION_Y_MIN) cy = BOARD_COLLISION_Y_MIN;
    int col = cx / 8 - 1;
    int row = cy / 8 - 2;
    return row * BOARD_COLLISION_ROW_STRIDE + col;
}

static void test_basic_span_and_isolation(void)
{
    const char *t = "basic_span_and_isolation";
    reset_grid();

    /* x=64,y=32,h=16 -> col=64/8-1=7, row=32/8-2=2, offset=2*38+7=83;
     * row_end=(16+32-1)>>3=5, rows=5-4+1=2 -> spans offset 83 and 83+38=121. */
    int off0 = span_offset(64, 32);
    int off1 = off0 + BOARD_COLLISION_ROW_STRIDE;
    check(t, off0 == 83 && off1 == 121, "hand-computed offsets must match the routine's own formula");

    g_grid[off0] = 0x01;
    g_grid[off1] = 0x02;
    /* Neighbours just outside the 2-row span must not affect the result. */
    g_grid[off0 - 1] = (dos_char)0xFF;
    g_grid[off0 + 1] = (dos_char)0xFF;
    g_grid[off1 + BOARD_COLLISION_ROW_STRIDE] = (dos_char)0xFF;

    dos_int r = board_collision_span_or(64, 32, 16);
    check(t, r == 0x03, "result must OR exactly the two in-span bytes and nothing else");
}

static void test_x_y_clamping(void)
{
    const char *t = "x_y_clamping";
    reset_grid();

    /* x, y both driven far outside their clamp ranges; h=1 keeps the row
     * span to exactly the clamped row so the test isolates the clamp. */
    int cell = span_offset(-1000, -1000); /* clamps to (8, 0x10) */
    check(t, cell == span_offset(8, 0x10), "very negative x/y must clamp the same as the clamp bounds themselves");
    g_grid[cell] = 0x40;
    dos_int r = board_collision_span_or(-1000, -1000, 1);
    check(t, r == 0x40, "out-of-range x/y must clamp to [8,0x137]/[0x10,0x9F] before addressing the grid");

    reset_grid();
    int off2 = span_offset(30000, 30000); /* clamps to (0x137, 0x9F) */
    check(t, off2 == span_offset(0x137, 0x9F), "very large x/y must clamp the same as the clamp bounds themselves");
    g_grid[off2] = 0x40;
    r = board_collision_span_or(30000, 30000, 1);
    check(t, r == 0x40, "very large x/y must clamp to the upper bounds before addressing the grid");
}

static void test_row_end_clamp(void)
{
    const char *t = "row_end_clamp";
    reset_grid();

    /* x=8 (min), y=0x10 (min), h huge -> row_end clamps to
     * BOARD_COLLISION_MAX_ROW_END(0x13); row_start = 0x10/8-... wait: the
     * routine's row_start is y>>3 (=2) used for row_base = row_start-2 =
     * 0, and rows = row_end_clamped - (y>>3) + 1 = 0x13-2+1 = 18. */
    dos_int col = (dos_int)(BOARD_COLLISION_X_MIN / 8 - 1); /* 0 */
    dos_int row_base = 0;
    int rows = BOARD_COLLISION_MAX_ROW_END - (BOARD_COLLISION_Y_MIN / 8) + 1;
    check(t, rows == 18, "row-count arithmetic sanity check");

    int last_off = row_base * BOARD_COLLISION_ROW_STRIDE + col + (rows - 1) * BOARD_COLLISION_ROW_STRIDE;
    check(t, (size_t)(last_off + 1) <= sizeof g_grid, "test grid must be large enough for the clamped span");
    g_grid[last_off] = 0x08; /* only the LAST row of the clamped span is set */

    dos_int r = board_collision_span_or(BOARD_COLLISION_X_MIN, BOARD_COLLISION_Y_MIN, 5000);
    check(t, r == 0x08, "an enormous h must clamp the row span, not read past BOARD_COLLISION_MAX_ROW_END rows");
}

static void test_zero_extended_return(void)
{
    const char *t = "zero_extended_return";
    reset_grid();

    int cell = span_offset(64, 32);
    g_grid[cell] = (dos_char)0x80; /* high bit set: must NOT sign-extend on return */
    dos_int r = board_collision_span_or(64, 32, 1);
    check(t, r == 0x80, "return value must be the zero-extended byte (AH cleared by `xor ax,ax`), not sign-extended");
}

int main(void)
{
    test_basic_span_and_isolation();
    test_x_y_clamping();
    test_row_end_clamp();
    test_zero_extended_return();

    if (g_failures) {
        fprintf(stderr, "test_asm_boardcol: %d failure(s)\n", g_failures);
        return 1;
    }
    printf("test_asm_boardcol: OK\n");
    return 0;
}
