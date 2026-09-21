/* asm_iconanim.h -- icon animation record traversal, asm/ICONANIM.ASM
 * (`_icon_list_animate_draw` F_D386, `_icon_frame_reset_and_draw` F_D3CF).
 *
 * List: `icon_record_list_ptr` (DS:BFC4, generated `dos_uchar
 * *icon_record_list_ptr`, portable/generated/game_state.h), set by
 * src/BOARD.C (`icon_record_list_ptr = p;`, board_redraw_paint) to a
 * position inside `board_records`' nested tables, read by
 * `icon_list_animate_draw()` in src/GAME.C's main loop
 * (`if (*icon_record_list_ptr != 0) icon_list_animate_draw();`).
 *
 * Layout: outer byte = record count, then exactly `count` FIXED-size
 * 12-byte records (asm-module-inventory.md sec 6 described this as
 * "variable-length" -- that is wrong; disproved directly by the ASM's own
 * `mov si,di; add si,0Ch` at the bottom of every loop iteration, which
 * unconditionally advances to `record_start + 12` regardless of anything
 * read from the record.  See the port report for the full trace).  Each
 * 12-byte record:
 *
 *   byte0        = cursor        (0..8: index into the frames[] payload
 *                                  below, read and then usually incremented
 *                                  every call this record is visited)
 *   byte1..byte2 = xy (LE word)   byte1 = x low byte (doubled for the draw
 *                                  x), byte2 = y low byte (used as-is)
 *   byte3..byte11 = frames[9]     a 9-byte animation-frame-id sequence;
 *                                  the record's cursor selects which byte
 *                                  of this array is used to pick this
 *                                  call's sprite
 *
 * Per call: `frame_raw = record.frames[record.cursor]`.
 *   - if `frame_raw - 1 >= 0` (i.e. frame_raw >= 1, signed byte): this is
 *     the "advance" path -- draw a72b2[frame_raw - 1], then
 *     `record.cursor += 1` (persisted for the next call/frame).
 *   - else (frame_raw == 0, the sequence's terminator/loop-point): this is
 *     the "reset" path (`_icon_frame_reset_and_draw`'s own body) --
 *     `record.cursor` is SET to 1 (not incremented), and THIS call draws
 *     a72b2[record.frames[0]] directly (the raw byte, no -1 adjustment --
 *     frames[0] is evidently pre-biased by the data format's own
 *     convention for the loop-start frame).
 * `a72b2` (DS:72B2, generated `dos_char *a72b2[40]`) is indexed directly
 * by the resolved id (`(id&0xff)<<2` in the ASM, i.e. plain array
 * indexing here) and the far pointer handed to gfx_blit_bitmap.
 *
 * Draw: `gfx_blit_bitmap(x = record.xy_lo * 2, y = record.xy_hi,
 * a72b2[index])` -- traced from the ASM's `xchg dl,ch` / `shl cx,1` /
 * push order exactly as asm_drawq.c's sibling trace was (same halved-x,
 * unscaled-y convention as the rest of this port).
 *
 * `_icon_frame_reset_and_draw` is public but, per the inventory's own
 * grep and this port's independent check, has no C caller anywhere and is
 * only reachable as a same-iteration fall-through from
 * `_icon_list_animate_draw`'s own DI/SI-positioned state (the ASM file's
 * own header comment says as much).  This port therefore inlines its
 * logic into `icon_list_animate_draw()`'s loop body (the "reset path"
 * above) instead of expressing it as a real call; the exported
 * `icon_frame_reset_and_draw(void)` (game_funcs.h) is kept only because
 * that prototype already exists there, and is a documented no-op -- see
 * its definition in asm_iconanim.c.
 */
#ifndef PORTABLE_ASM_ICONANIM_H
#define PORTABLE_ASM_ICONANIM_H

#include "dos_types.h"

#define ICON_ANIM_RECORD_BYTES  12u
#define ICON_ANIM_PAYLOAD_BYTES 9u  /* record.frames[], at record offset 3 */

#endif /* PORTABLE_ASM_ICONANIM_H */
