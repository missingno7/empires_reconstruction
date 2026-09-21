/* asm_boardcol.c -- semantic port of asm/BOARDCOL.ASM:
 * _board_collision_span_or (F_1F91).
 *
 * See asm_boardcol.h for the grid geometry.  Steps, matching the ASM
 * exactly (comments cite the instructions):
 *
 *   1. Clamp x to [8, 0x137], y to [0x10, 0x9F] (signed compares, as the
 *      ASM's `jg`/`jge` are).
 *   2. col = (x >> 3) - 1.
 *   3. row_end = (h + y - 1) >> 3, then clamped to a max of 0x13 (19).
 *      Built from the UNSHIFTED clamped y (the ASM computes `cx = h + bx`
 *      before `bx` itself is shifted down to y/8).
 *   4. row_start = y >> 3; rows = row_end - row_start + 1;
 *      row_base = row_start - 2.
 *   5. offset = row_base * 0x26 + col; walk `rows` bytes of board_records
 *      starting at board_records + offset, stride 0x26, OR-ing them all
 *      together; return the accumulated byte (zero-extended, matching
 *      `xor ax,ax` before the loop -- only AL is ever written).
 *
 * Same semantic hazard as asm_rectq.c: the row loop is a bare x86 `loop`
 * with no zero-guard (`xor ax,ax` falls straight into `@@row:`), so if
 * `rows` ever computed to 0 mod 65536 the historical routine would OR
 * 65536 far bytes.  Preserved below as a do/while.  Not reachable by
 * either real caller: src/GAME.C and src/LEVEL.C only ever pass h in
 * {0x27, 0x1d} (39, 29), which keeps row_end >= row_start for every y in
 * the clamped range -- flagged for completeness, not a live bug.
 */
#include "game.h"
#include "asm_boardcol.h"

dos_int board_collision_span_or(dos_int x, dos_int y, dos_int h)
{
    dos_int cx_; /* clamped x */
    dos_int cy;  /* clamped y */
    dos_int col;
    dos_int row_end;
    dos_uint row_end_u;
    dos_int row_start;
    dos_int row_base;
    dos_int offset;
    const dos_char *p;
    dos_uint rows;
    dos_uchar acc;

    cx_ = x;
    if (cx_ > BOARD_COLLISION_X_MAX)      cx_ = BOARD_COLLISION_X_MAX;
    else if (cx_ < BOARD_COLLISION_X_MIN) cx_ = BOARD_COLLISION_X_MIN;

    cy = y;
    if (cy > BOARD_COLLISION_Y_MAX)      cy = BOARD_COLLISION_Y_MAX;
    else if (cy < BOARD_COLLISION_Y_MIN) cy = BOARD_COLLISION_Y_MIN;

    col = (dos_int)((dos_uint)cx_ >> 3); /* shr ax,1 x3 */

    row_end = dos_sub16(dos_add16(h, cy), 1); /* cx = h + bx (unshifted y); dec cx */
    row_end_u = (dos_uint)row_end;
    row_end_u >>= 3;                          /* shr cx,1 x3 (logical, like the CPU) */
    row_end = (dos_int)row_end_u;
    if (row_end >= BOARD_COLLISION_MAX_ROW_END) row_end = BOARD_COLLISION_MAX_ROW_END; /* cmp cx,13h; jl */

    row_start = (dos_int)((dos_uint)cy >> 3); /* shr bx,1 x3 */

    rows = (dos_uint)dos_add16(dos_sub16(row_end, row_start), 1); /* sub cx,bx; inc cx */

    row_base = dos_sub16(row_start, 2); /* sub bx,2 */
    col = dos_sub16(col, 1);            /* dec ax */

    offset = dos_add16(dos_mul16(row_base, BOARD_COLLISION_ROW_STRIDE), col);
    p = board_records + offset;

    acc = 0;
    do {
        acc |= (dos_uchar)*p;             /* or al,[si] */
        p += BOARD_COLLISION_ROW_STRIDE;  /* add si,26h */
        rows = (dos_uint)(rows - 1);      /* loop @@row */
    } while (rows != 0);

    return (dos_int)acc;
}
