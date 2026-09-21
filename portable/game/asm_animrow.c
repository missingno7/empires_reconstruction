/* asm_animrow.c -- semantic port of asm/ANIMROW.ASM: _anim_step_row_copy
 * (F_9EC3).  See asm_animrow.h for the seed/chain table this routine
 * chains through.
 *
 * game_funcs.h's existing prototype (asm-module-inventory.md sec 5's own
 * two unnamed "?" args) names its parameters as if bp+4/bp+0Ch were row
 * indices and bp+6/bp+0Eh were offsets; this port's read of the ASM shows
 * the opposite pairing, and that arg6/arg8 (its bp+0Eh/bp+12h) are in fact
 * the destination row index and the video-mode row stride, while bp+10h
 * (labelled `row_stride` there) is actually the 0..15 animation-step
 * index.  Types and argument COUNT/ORDER already match; only the names
 * were a placeholder -- see this module's port report.  The real roles,
 * positionally identical to game_funcs.h's `anim_step_row_copy`:
 *
 *   bp+4  src_off      (ASM's `a`)  byte offset into the source row
 *   bp+6  src_row_idx  (`b`)        g3924[] index of the source row
 *   bp+8  width        (`q`)       bytes to copy per row
 *   bp+0A rows         (`d`)        outer row-loop trip count
 *   bp+0C dst_off      (`e`)       byte offset into the destination row
 *   bp+0E dst_row_idx  (`f`)        g3924[] index of the destination row
 *   bp+10 step_index   (`i`)        0..15 animation step (xlat seed index)
 *   bp+12 row_stride                video-mode row width: 0x140/0x50/0xA0
 *
 * (src/ANIMSTEP.C call shape: `anim_step_row_copy(a,b,q,d,e,f,i,stride)`,
 * three variants scaled by the active display_mode's pixels-per-byte.)
 *
 * Per-row copy algorithm (see asm_animrow.h for the "why"): each row is
 * split into ANIMROW_XLAT_BLOCK_BYTES(16)-byte blocks; per block, exactly
 * one byte -- at the offset the seed/chain tables select -- is copied,
 * the rest of the block is left untouched.  The block offset starts at
 * chain[seed[step_index]] and advances by one more chain[] step per row
 * (including row 0).
 */
#include "game.h"
#include "asm_animrow.h"

void anim_step_row_copy(dos_int src_off, dos_int src_row_idx, dos_int width,
                         dos_int rows, dos_int dst_off, dos_int dst_row_idx,
                         dos_int step_index, dos_int row_stride)
{
    uint8_t *dst = g3924[(dos_uint)dst_row_idx] + dst_off; /* les di,g3924[f*4]; add di,e */
    uint8_t *src = g3924[(dos_uint)src_row_idx] + src_off; /* lds si,g3924[b*4]; add si,a */
    dos_int row_skip = dos_sub16(row_stride, width);       /* ax=stride; ax-=q -> [bp-2] */
    dos_uchar seg = animrow_xlat_seed((dos_uchar)step_index); /* ax=i; xlat DS:12B0h */
    dos_uint row_count = (dos_uint)rows;                   /* cx = d */

    /* loop F9_row has no cx==0 guard either (falls straight in from the
     * seed xlat above); same 65536-iteration hazard as asm_rectq.c /
     * asm_boardcol.c if a caller ever passed rows==0.  Neither
     * src/ANIMSTEP.C call site can: `d` is anim_step_loop's own row-count
     * argument and both call sites pass a fixed positive constant. */
    do {
        dos_int remaining = width; /* dx = q, (re)loaded each row */

        seg = animrow_xlat_chain(seg); /* F9_row: xlat DS:12C0h, chained every row incl. row 0 */

        while (remaining > (dos_int)seg) { /* F9_segment: cmp dx,ax; jng F9_tail */
            src += seg; /* add si,ax */
            dst += seg; /* add di,ax */
            *dst = *src; /* movsb: the one byte this block reveals */
            src += 1;    /* movsb auto-increment (cld => forward) */
            dst += 1;
            src += (dos_int)(ANIMROW_XLAT_BLOCK_BYTES - 1u) - (dos_int)seg; /* add si,0Fh; sub si,ax */
            dst += (dos_int)(ANIMROW_XLAT_BLOCK_BYTES - 1u) - (dos_int)seg; /* add di,0Fh; sub di,ax */
            remaining -= (dos_int)ANIMROW_XLAT_BLOCK_BYTES;                /* sub dx,10h */
        } /* jg F9_segment folded into the while condition */

        src += remaining; /* F9_tail: add si,dx (dx may be <= 0 here) */
        dst += remaining; /* add di,dx */
        src += row_skip;  /* add si,[bp-2] */
        dst += row_skip;  /* add di,[bp-2] */

        row_count = (dos_uint)(row_count - 1); /* loop F9_row */
    } while (row_count != 0);
}
