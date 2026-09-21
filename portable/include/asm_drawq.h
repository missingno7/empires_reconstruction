/* asm_drawq.h -- render an ES:DI command list, asm/DRAWQ.ASM
 * (`_draw_queue_render_highlighted` F_D61C, `_draw_queue_render` F_D79C).
 *
 * *** Corrects asm-module-inventory.md sec 7/summary ***  The inventory
 * flagged DS:0BFC0h ("les di,dword ptr ds:[0bfc0h]") as having "no
 * C-facing name or write site found" and guessed it was "very plausibly"
 * asm/DRAWQBUF.ASM's DS:2F30h queue.  It is neither: DS:0BFC0h IS a named,
 * generated DGROUP object -- `record_table_root` (state-map.md, generated
 * `dos_char *record_table_root`) -- and its one and only write site,
 * src/BOARD.C:436 (`record_table_root = (char far *)(board_records +
 * 0x2ca);`), points into `board_records` (the loaded level/map buffer),
 * NOT at g2f30.  Proof beyond the write site: DRAWQ.ASM's own record loop
 * advances its cursor by exactly 4 bytes per record (`mov bx,es:[di];
 * inc di; inc di; mov cx,es:[di]; inc di; ...; inc di` = +2+1+1), but
 * DRAWQBUF.ASM's producer writes 5-byte records (byte+word+word). A
 * 4-byte consumer cannot be draining a 5-byte producer's queue. The
 * 4-byte stride instead matches src/BOARD.C's OWN arithmetic on this same
 * pointer elsewhere in the file (`record_table_level4_ptr`:
 * `record_table_root + *record_table_root*4 + 1`, and F_32FA's
 * `record_table_root+a*4+1`) -- i.e. DRAWQ.ASM and BOARD.C's C code are
 * two different consumers of the same 4-byte-record table living inside
 * `board_records`.  asm/DRAWQBUF.ASM's g2f30 queue appears, on the
 * evidence available to this port, to have no consumer at all (see
 * asm_drawqbuf.h).
 *
 * Record format (4 bytes), matching the ASM's own di+=4-per-record walk:
 *   byte0 = attr            (becomes the render x position, attr*2)
 *   byte1 = w1_lo            (becomes the render y position, unscaled)
 *   byte2 = w1_hi            (becomes the "strip" repeat count, 0..255)
 *   byte3 = color            (selects one of 8 g2380 colour slots, and
 *                             is REWRITTEN in place every render -- see
 *                             below)
 *
 * Bitmap source: g2380 (DS:2380, generated `dos_char g2380[23][130]` --
 * 2990 bytes, matching src/BOARD.C's own `extern char g2380[][0x82];`
 * declaration; asm_drawq.c addresses it through a flat byte view since
 * DRAWQ.ASM's own index arithmetic is a raw displacement, not a row/col
 * pair) is unpacked by src/BOARD.C's `resource_scoreboard_unpack` (F_2119) as 8
 * consecutive 0x176(374)-byte "colour slots" (2992 = 8*374), each slot
 * itself 3 sub-images back-to-back: a 0x82(130)-byte "left cap", a
 * 0x62(98)-byte "middle tile", and a 0x92(146)-byte "right cap"
 * (0x82+0x62+0x92 == 0x176).  DRAWQ.ASM selects a slot with
 * `sign_extend(record.color) * 0x176`, draws the left cap with
 * gfx_copy_rect, the middle tile `strip_count` times with gfx_blit_bitmap
 * (x advancing +8 each time, first at x+0x0C), then the right cap with
 * gfx_copy_rect -- the classic left-cap/repeated-middle/right-cap "bar"
 * layout (this looks like a per-owner-colour scoreboard/status bar; not
 * confirmed beyond the resource_scoreboard_unpack naming and the 8-slot,
 * 3-chunk shape -- report as inferred, not certain).
 *
 * In-place record mutation (real, NOT dead code): after reading
 * record.color for this frame's slot selection, both render routines
 * unconditionally rewrite the record's 4th byte: += 1, except when
 * (color & 3) == 3, where it is -= 3 instead.  Applied every frame to the
 * SAME byte (`record_table_root`'s target persists across calls -- it is
 * not a per-frame scratch buffer, unlike g2f30), this cycles the byte
 * through exactly 4 values forever (e.g. 4,5,6,7,4,5,6,7,...): a
 * deliberate colour-cycle/"shimmer" animation for whichever slot each
 * record currently selects.  Ported as a real write into the caller's
 * buffer -- `draw_queue_render_list` below therefore takes a *mutable*
 * `uint8_t *list`, not the `const uint8_t *` a register-ABI cleanup would
 * normally suggest (see this module's port report for why the `const`
 * shape was rejected).
 *
 * Highlighted-only extras (`_draw_queue_render_highlighted`):
 *   - widens the row clip (`g96 = 0x190` on entry, `g96 = 0x9F` on exit --
 *     both literal restores, not save/restore of a prior value);
 *   - adds 0xB8 to every record's render y (the D61 vs D79 instruction
 *     streams are otherwise identical -- this offset is easy to miss
 *     because it is folded into a `mov ah,dl / mov al,bh / add ax,0b8h`
 *     sequence that has no separate label);
 *   - before/after the record loop, pokes 24 fixed byte offsets into
 *     g2380 to 0x10 (entry) / 0x0B (exit).  These 24 offsets step through
 *     the pattern +0x82,+0x62,+0x92 repeating (the exact 3 chunk sizes
 *     above) starting at 0x21 -- i.e. one fixed byte inside EACH of the 8
 *     colour slots' 3 sub-images (8*3 = 24): a shared "highlight pixel"
 *     baked into every slot/chunk's bitmap data, brightened while the
 *     highlighted pass renders and restored after.  (This is a different,
 *     unrelated interpretation from asm-module-inventory.md sec 7's
 *     guess that these offsets index `g2380[][0x82]`'s "24-cell board
 *     lattice" rows -- they are literal byte offsets into the same flat
 *     g2380 array DRAWQ.ASM's own record loop reads pixels from, not a
 *     row-major cell table; src/BOARD.C separately declares `g2380[][0x82]`
 *     for ITS OWN purposes, but that 2D view is not what these 24 fixed
 *     constants are indexing.)
 */
#ifndef PORTABLE_ASM_DRAWQ_H
#define PORTABLE_ASM_DRAWQ_H

#include "dos_types.h"

#define DRAW_QUEUE_RENDER_RECORD_BYTES 4u

#define DRAW_QUEUE_RENDER_SLOT_BYTES        0x176u /* one g2380 colour slot */
#define DRAW_QUEUE_RENDER_LEFT_CAP_BYTES    0x82u
#define DRAW_QUEUE_RENDER_MIDDLE_TILE_BYTES 0x62u
#define DRAW_QUEUE_RENDER_RIGHT_CAP_BYTES   0x92u

/* Testable core both draw_queue_render() and draw_queue_render_highlighted()
 * (asm_drawq.c) forward to, taking the list explicitly instead of reading
 * record_table_root itself -- exercised directly by portable/tests/
 * test_asm_drawq.c on synthetic records.  `list[0]` is the record count;
 * `list` is mutated in place per record (see the file header above). */
void draw_queue_render_list(uint8_t *list, dos_int highlighted);

#endif /* PORTABLE_ASM_DRAWQ_H */
