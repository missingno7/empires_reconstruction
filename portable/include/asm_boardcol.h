/* asm_boardcol.h -- collision/attribute grid geometry, asm/BOARDCOL.ASM
 * (`_board_collision_span_or`, F_1F91).
 *
 * board_records (DS:BFBC, generated `dos_char *`, portable/generated/
 * game_state.h) is a row-major byte grid, BOARD_COLLISION_ROW_STRIDE
 * (0x26 = 38) bytes per row.  The routine ORs together every grid byte
 * the caller's (x,y,h) span covers, after:
 *   - x clamped to [BOARD_COLLISION_X_MIN, BOARD_COLLISION_X_MAX], then
 *     divided by 8 to a column cell (x/8 - 1 is the actual column used);
 *   - y clamped to [BOARD_COLLISION_Y_MIN, BOARD_COLLISION_Y_MAX], then
 *     divided by 8 to a row cell (y/8 - 2 is the actual row used);
 *   - the row span (h rows starting at the clamped y) further clamped so
 *     its end cell never exceeds BOARD_COLLISION_MAX_ROW_END.
 * This is the same far `board_records` block asm/SPRITES.ASM indexes with
 * a *different* (0x20-byte-record) scheme for actor lookups -- here it is
 * addressed directly as the collision/attribute grid, one byte per cell.
 */
#ifndef PORTABLE_ASM_BOARDCOL_H
#define PORTABLE_ASM_BOARDCOL_H

#include "dos_types.h"

#define BOARD_COLLISION_ROW_STRIDE  0x26 /* 38 */
#define BOARD_COLLISION_X_MIN       8
#define BOARD_COLLISION_X_MAX       0x137
#define BOARD_COLLISION_Y_MIN       0x10
#define BOARD_COLLISION_Y_MAX       0x9F
#define BOARD_COLLISION_MAX_ROW_END 0x13 /* 19 */

#endif /* PORTABLE_ASM_BOARDCOL_H */
