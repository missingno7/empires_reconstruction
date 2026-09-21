/* asm_drawqbuf.c -- semantic port of asm/DRAWQBUF.ASM:
 * _draw_queue_reset (F_D818), _draw_queue_append (F_D825).
 *
 * See asm_drawqbuf.h for the g2f30 buffer/record layout and the (negative)
 * finding that nothing in the historical tree was found to read it back.
 *
 * _draw_queue_append's parameter count/order was flagged AMBIGUOUS by
 * asm-module-inventory.md sec 8; resolved here directly from the ASM body
 * (all five stack words [bp+4]..[bp+12] are read) and cross-checked
 * against both real callers: src/BOARD.C's own
 * `extern void draw_queue_append(char, int, int, int, int);` (5 call
 * sites, e.g. `draw_queue_append(i + 1, w8, j, 8, 0x10)`) and
 * asm/SPRDRAW.ASM's `_sprite_table_queue_draws` ASM-to-ASM call, which
 * pushes `ax=0x1E(height), ax=0x0F(color), dx(y), bx(x), bp(id/attr)` in
 * that order -- last-pushed (bp) ends up at [bp+4], so the parameter order
 * is (attr, x, y, color, height), matching BOARD.C's call shape exactly
 * and matching game_funcs.h's already-published prototype.
 */
#include "game.h"
#include "asm_drawqbuf.h"

void draw_queue_reset(void)
{
    g2f30[0] = 0; /* mov di,2f30h; mov byte ptr [di],0 */
}

void draw_queue_append(dos_char attr, dos_int x, dos_int y, dos_int color, dos_int height)
{
    dos_uchar count = g2f30[0];                 /* mov bl,[di] */
    /* al=bl; shl al,1; shl al,1; add al,bl -- an 8-BIT (not 16-bit) multiply
     * by 5; the ASM zero-extends the truncated byte into AX afterwards
     * (`sub ah,ah`), so a count >= 52 wraps the record offset within the
     * byte before it is added to DI.  Preserved exactly via the dos_uchar
     * truncation below; g2f30 is only 162 bytes (32 records) so no real
     * caller can reach this, same class of preserved-but-unreachable
     * hazard as asm_boardcol.c/asm_rectq.c. */
    dos_uchar record_off = (dos_uchar)(count * 5u);
    dos_uchar *rec = &g2f30[1] + record_off;      /* inc di; add di,ax */

    g2f30[0] = (dos_uchar)(count + 1u);           /* inc byte ptr [di] (the ORIGINAL di, before the advance above) */

    rec[0] = (dos_uchar)attr;                     /* mov al,[bp+4]; stosb */
    rec[1] = dos_lo8((uint16_t)x);                 /* mov ax,[bp+6]; mov ah,bl(=y lo) -- al half */
    rec[2] = dos_lo8((uint16_t)y);                 /* stosw high half */
    rec[3] = dos_lo8((uint16_t)color);             /* mov ax,[bp+10]; mov ah,bl(=height lo) -- al half */
    rec[4] = dos_lo8((uint16_t)height);            /* stosw high half */

    /* AX (the ASM's own return value, "address of the record's final
     * word") is not modelled: game_funcs.h publishes this as `void`
     * because every real C caller (BOARD.C) discards it. */
}
