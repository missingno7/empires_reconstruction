/* asm_drawqbuf.h -- packed record layout for the DS:2F30h queue,
 * asm/DRAWQBUF.ASM (`_draw_queue_reset` F_D818, `_draw_queue_append` F_D825).
 *
 * Storage: g2f30 (DS:2F30, generated `dos_uchar g2f30[162]`, portable/
 * generated/game_data.h) -- a *fixed* near DGROUP buffer, not a far-pointer
 * cursor queue like RECTQ.ASM's.  Layout: g2f30[0] is the record count
 * (byte), then up to 32 five-byte records starting at g2f30[1] (1 + 32*5 =
 * 161; the generated region's 162nd byte is trailing DATA_011FB0_ZERO_REGION
 * pad, matching the ASM file's own "word alignment" header comment).
 *
 * Record (5 bytes), written by `stosb`/`stosw`/`stosw` in that order:
 *   byte0        = attr            (the caller's `attr` argument, low byte)
 *   byte1..byte2 = w1 (LE word)     byte1 = x low byte, byte2 = y low byte
 *   byte3..byte4 = w2 (LE word)     byte3 = color low byte, byte4 = height low byte
 * i.e. `draw_queue_append(attr, x, y, color, height)` truncates every
 * argument but `attr` to its low 8 bits before packing (`mov ah,bl` /
 * `stosw` only ever stores AL/AH -- never a full 16-bit argument).
 *
 * IMPORTANT: despite the superficial resemblance (5-byte packed records,
 * a leading count byte) this is a *different* buffer and record format
 * from the one asm/DRAWQ.ASM actually renders.  See asm_drawq.h / this
 * module's port report: DRAWQ.ASM's `les di,ds:[0bfc0h]` resolves to
 * `record_table_root` (DS:BFC0, generated `dos_char *record_table_root`),
 * which src/BOARD.C sets to `board_records + 0x2ca` -- never to
 * `&g2f30[0]`.  No consumer of g2f30 was found anywhere in asm/*.ASM or
 * src/*.C during this port (grep-confirmed); it is written every board
 * redraw (src/BOARD.C's `draw_queue_reset()` + repeated
 * `draw_queue_append()` calls, plus asm/SPRDRAW.ASM's ASM-to-ASM call from
 * `_sprite_table_queue_draws`) but, on the evidence available to this
 * scan, never read back by anything in the historical tree.  Ported
 * faithfully regardless (a real DGROUP buffer with real, observable
 * writes) -- flagged for Wave-3 review in case a consumer exists outside
 * asm/DRAWQ.ASM, asm/DRAWQBUF.ASM, src/BOARD.C, src/GAME.C (e.g.
 * RUNTIME_BLOCK.ASM, or a module this scan did not cover).
 */
#ifndef PORTABLE_ASM_DRAWQBUF_H
#define PORTABLE_ASM_DRAWQBUF_H

#include "dos_types.h"

#define DRAW_QUEUE_APPEND_RECORD_BYTES 5u
#define DRAW_QUEUE_APPEND_MAX_RECORDS  32u /* (sizeof g2f30 - 1) / 5, no bound enforced (matches the ASM: it never checks) */

#endif /* PORTABLE_ASM_DRAWQBUF_H */
