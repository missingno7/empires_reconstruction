/* shbmphit.c -- src/SHBMPHIT.C: collision probe against the shadow bitmap
 * (`vram`, a #define alias for board_records -- game_state.h).
 *
 * F_1F17 -- PORT: transcribed whole-body from the inline asm (the
 * historical comment notes TC 2.01 already saves si/di itself for this
 * function, so only push/pop ds bracket the body -- neither is portable
 * segment state, both dropped).  Traced instruction-by-instruction below;
 * the two clamp blocks, the byte-index math (matching BOARD.C's
 * `(x>>3)+((y>>3)-2)*38-1` cell-index formula -- int-semantics-inventory.md
 * 2.2's BOARD.C:279,344 entry), and the final accumulation loop are each
 * kept as a distinct, literal step rather than algebraically simplified,
 * so the byte-significant intermediate truncations survive.
 */
#include "game.h"

dos_int shadow_bitmap_hit_test(dos_int x, dos_int y, dos_int w)
{
    dos_int clamp_x, clamp_y;
    dos_uint x8;             /* ax after the three `shr ax,1`, i.e. clamp_x>>3 */
    dos_uint y8;              /* bx after its three shr's minus 2 */
    dos_uint col_count;        /* cx: byte span to OR together (may be 0,
                                 * meaning 65536 per the `loop` instruction's
                                 * decrement-then-test-nonzero semantics --
                                 * modelled literally below, not clamped to
                                 * >=1, since that is what the asm does) */
    dos_uint offset;
    dos_uchar *si;
    dos_uchar acc;

    /* L1/L2: clamp_x = x clamped to [8, 0x137]; L3/L4: clamp_y = y clamped
     * to [0x10, 0x9f].  Both compares are signed (`cmp ax,..`/`jg`/`jge`
     * on plain int parameters). */
    if (x > 0x137)
        clamp_x = 0x137;
    else if (x >= 8)
        clamp_x = x;
    else
        clamp_x = 8;

    if (y > 0x9f)
        clamp_y = 0x9f;
    else if (y >= 0x10)
        clamp_y = y;
    else
        clamp_y = 0x10;

    /* ax = clamp_x>>1; cx = w+ax-1; ax = ax>>1>>1 (clamp_x>>3); cx = cx>>1>>1;
     * cx = min(cx, 0x26); cx = cx - ax (ax still clamp_x>>3, not yet
     * decremented) + 1. */
    x8 = (dos_uint)clamp_x >> 3;
    {
        dos_uint half = (dos_uint)clamp_x >> 1;
        dos_uint t = (dos_uint)((dos_uint)w + half - 1);
        t >>= 2;
        if (t >= 0x26)
            t = 0x26;
        col_count = (dos_uint)(t - x8 + 1);
    }

    /* bx = clamp_y>>3 - 2; ax-- (ax = clamp_x>>3 - 1); bx = bx*38 (via the
     * shl/add sequence: bx*2, +bx*4 => *6, then *8 => *48, +*6 => *38's
     * factorisation as 6+32); bx += ax; si = vram + bx. */
    y8 = ((dos_uint)clamp_y >> 3) - 2;
    x8 = x8 - 1;   /* the `dec ax` that happens after col_count is computed */
    offset = (dos_uint)(y8 * 38 + x8);

    si = (dos_uchar *)vram + offset;

    /* xor ax,ax; L6: or al,[si]; inc si; loop L6 -- x86 LOOP decrements cx
     * THEN tests, so the body runs once unconditionally and a col_count of
     * 0 wraps to 65536 iterations; a plain `for (k=0;k<col_count;k++)`
     * would instead run zero times for that case, so this is written as
     * the equivalent post-test loop. */
    acc = 0;
    do {
        acc |= (dos_uchar)*si++;
        col_count = (dos_uint)(col_count - 1);
    } while (col_count != 0);

    return acc;
}
