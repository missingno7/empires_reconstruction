/* asm_rectq.c -- semantic port of asm/RECTQ.ASM: _rect_queue_flush (F_1ECD).
 *
 * The ASM drains the 4-byte dirty-rect records between the read cursor
 * ui_gfx_blob and the write cursor rect_queue_write_ptr, calling
 * gfx_box(x,y,w,h) for each, then resets rect_queue_write_ptr back to
 * ui_gfx_blob (queue emptied).  See asm_rectq.h for the record layout and
 * the storage trail (no dedicated DGROUP buffer -- the queue reuses
 * portable/resource's decode-staging memory via ui_gfx_blob).
 *
 * g40c6 (the historical segment half of rect_queue_write_ptr, DS:40C6) is
 * intentionally not modelled: state-map.md records it as "no portable
 * object" now that far pointers are flat pointers (architecture.md,
 * "Shared game state").
 *
 * IMPORTANT semantic hazard, faithfully preserved from the ASM: the drain
 * loop has NO zero-count guard.  `les cx,rect_queue_write_ptr; lds
 * si,ui_gfx_blob; sub cx,si; shr cx,1; shr cx,1` falls straight into the
 * loop body with no `jcxz`; if the queue is already empty (write pointer
 * == read pointer), cx computes to 0 and the body still runs once, then
 * x86's `loop` decrements cx to 0xFFFF and keeps going -- 65536 iterations
 * walking rect_queue_write_ptr/ui_gfx_blob's shared buffer far out of
 * bounds.  Ported below as the same do/while-with-post-decrement so the
 * observable behaviour for any well-formed (non-empty) call is identical;
 * callers MUST NOT call rect_queue_flush() on an empty queue.  This is not
 * theoretical: src/LEVEL.C's own call sites are inconsistent about it --
 * line 175 guards with `if (rect_queue_write_ptr != ui_gfx_blob)
 * rect_queue_flush();`, but lines 202 and 209 call it unconditionally,
 * relying on the preceding sprite_table_wipe_active()/
 * sprite_script_frame_driver() calls having appended at least one record.
 * See this module's port report for the full discrepancy.
 */
#include "game.h"
#include "asm_rectq.h"

void rect_queue_flush(void)
{
    const uint8_t *q = ui_gfx_blob;                 /* lds si, ui_gfx_blob */
    const uint8_t *wp = rect_queue_write_ptr;        /* les cx, rect_queue_write_ptr */
    dos_uint count = (dos_uint)((wp - q) / (ptrdiff_t)RECT_QUEUE_RECORD_BYTES); /* sub cx,si; shr;shr */

    do {
        dos_int x = (dos_int)(q[0] << 1); /* word1 lo = x/2; shl 1 restores x */
        dos_int y = (dos_int)q[1];        /* word1 hi = y, unshifted */
        dos_int w = (dos_int)(q[2] << 1); /* word2 lo = w/2; shl 1 restores w */
        dos_int h = (dos_int)q[3];        /* word2 hi = h, unshifted */
        q += RECT_QUEUE_RECORD_BYTES;

        gfx_box(x, y, w, h);

        count = (dos_uint)(count - 1); /* loop @@loop (post-decrement, wraps on 0) */
    } while (count != 0);

    rect_queue_write_ptr = ui_gfx_blob; /* les bx,ui_gfx_blob; mov rect_queue_write_ptr,bx */
}
