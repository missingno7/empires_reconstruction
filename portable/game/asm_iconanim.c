/* asm_iconanim.c -- semantic port of asm/ICONANIM.ASM:
 * _icon_list_animate_draw (F_D386), _icon_frame_reset_and_draw (F_D3CF).
 *
 * See asm_iconanim.h for the (corrected: fixed 12-byte, not variable-length)
 * record layout, the cursor/frames[] animation-cycle mechanism, and why
 * the "reset" routine's logic is inlined here rather than expressed as a
 * real standalone call.
 *
 * IMPORTANT semantic hazard, faithfully preserved from the ASM (same class
 * as asm_rectq.c/asm_boardcol.c/asm_drawq.c): the outer record loop has no
 * zero-count guard (`lodsb; xor cx,cx; mov cl,al` falls straight into
 * `fd386_next_record`, and the closing `loop` only stops after at least
 * one pass).  `icon_record_list_ptr[0] == 0` would process one bogus
 * record and then wrap the count to 0xFFFF.  The one real caller guards
 * against this itself (`if (*icon_record_list_ptr != 0)
 * icon_list_animate_draw();`, src/GAME.C) -- callers of this function
 * MUST NOT invoke it with `icon_record_list_ptr[0] == 0`.
 */
#include "game.h"
#include "asm_iconanim.h"

void icon_list_animate_draw(void)
{
    dos_uint count;
    dos_uchar *p; /* lds si, icon_record_list_ptr */

    if (icon_record_list_ptr == NULL)
        return; /* defensive; the ASM would fault on a null far pointer */

    count = icon_record_list_ptr[0]; /* fd386_read_record: lodsb; mov cl,al */
    p = icon_record_list_ptr + 1;

    do {
        dos_uchar *rec = p;               /* mov di,si (record start) */
        dos_uchar cursor = rec[0];        /* lodsb; mov bl,al */
        dos_uchar xy_lo = rec[1];         /* lodsw low byte */
        dos_uchar xy_hi = rec[2];         /* lodsw high byte */
        dos_uchar frame_raw;
        dos_int index;
        dos_int x, y;

        p += ICON_ANIM_RECORD_BYTES;      /* mov si,di; add si,0Ch (unconditional, fixed record size) */

        /* si = di+1(cursor)+2(xy)+cursor, then lodsb: reads rec[3+cursor],
         * one of the 9 frames[] bytes (cursor is bounds-trusted exactly
         * like the ASM trusts it -- no clamp is performed there either). */
        frame_raw = rec[3 + cursor];

        if ((dos_char)(frame_raw - 1) < 0) {
            /* _icon_frame_reset_and_draw's own body, inlined (see
             * asm_iconanim.h): "mov byte ptr [di],1" then re-read
             * rec[3+0] raw (no -1 adjustment). */
            rec[0] = 1;
            index = (dos_int)rec[3];
        } else {
            /* fd386_lookup via the "inc byte ptr [di]" fallthrough. */
            rec[0] = (dos_uchar)(cursor + 1u);
            index = (dos_int)(dos_uchar)(frame_raw - 1u);
        }

        x = (dos_int)(2u * (dos_uint)xy_lo); /* shl cx,1 (ch cleared by the xchg) */
        y = (dos_int)(dos_uint)xy_hi;        /* xchg dl,ch; push dx */

        /* `index` (0..255, from a raw payload byte) is used unclamped, same
         * as the ASM's own `(id&0xff)<<2` -- a72b2 has 40 entries, so a
         * record whose frames[]/frames[0] byte resolves to 40 or more reads
         * past a72b2 exactly as the historical code would.  Not verified
         * against real level data (icon_record_list_ptr's target format was
         * not decoded by this port); flagged in the port report. */
        gfx_blit_bitmap(x, y, (const uint8_t *)a72b2[index]); /* les si,[bx+_a72b2] */

        count = (dos_uint)(count - 1); /* loop fd386_next_record (post-decrement, wraps on 0) */
    } while (count != 0);
}

/* No C caller anywhere (grep-confirmed, matching asm-module-inventory.md
 * sec 6 and game_funcs.h's own note) and, per the ASM file's own header
 * comment, not independently reachable even from other ASM -- its body
 * only makes sense continuing icon_list_animate_draw's in-progress DI/SI
 * record walk, which this port inlines directly (see above).  Kept only
 * because game_funcs.h already publishes this zero-arg prototype;
 * documented no-op. */
void icon_frame_reset_and_draw(void)
{
}
