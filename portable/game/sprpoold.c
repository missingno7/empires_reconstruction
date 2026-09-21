/* sprpoold.c -- portable port of src/SPRPOOLD.C.
 *
 * F_78C3 -- draw up to four sprites from the pool at DS:C0EE, one per set
 * bit of mask.
 *
 * PORT / generator type issue: `dst` is a K&R `char near *` parameter, but
 * its one caller (src/SLOTROW.C: `sprite_pool_draw_masked(0xdd, y,
 * p->flags)`) passes a plain small integer (an x-coordinate), never a real
 * pointer -- game_funcs.h's mechanical extraction keeps the historical
 * (pointer-shaped) declaration anyway since that is what the DEFINITION
 * says.  Recovered here as the coordinate it actually is via a
 * pointer-to-integer round trip; the pointer value is never dereferenced.
 * gfx.h's real gfx_blit_bitmap(x,y,bitmap) confirms the coordinate reading
 * (SPRPOOLD.C's own local `extern void gfx_blit_bitmap(char near *,int,
 * char far *)` redeclaration was itself just as loosely typed).
 */
#include "game.h"

#include <stdint.h>

struct H {
    dos_char pad[0x28];
    dos_uint ofs[4];
};

void sprite_pool_draw_masked(dos_char *dst, dos_int b, dos_uint mask)
{
    dos_int x = (dos_int)(intptr_t)dst;
    dos_int k;

    for (k = 0; k < 4; k++)
        if (mask & (1 << k))
            gfx_blit_bitmap(x + k * 14, b,
                             (const uint8_t *)(gc0ee + ((const struct H *)gc0ee)->ofs[k] + 2));
}
