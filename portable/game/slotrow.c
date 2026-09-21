/* slotrow.c -- src/SLOTROW.C: draw one row of the save-slot list (name,
 * difficulty label, and either the score or a masked sprite-pool icon).
 */
#include "game.h"

static dos_char g1650[] = "Explorer";
static dos_char g1659[] = "Expert";

/* F_? -- p->text[0]==0 marks a free slot and draws nothing (falls straight
 * through; the historical function never returns a value on any path --
 * every caller (src/OPTIONS.C, src/SLOTMENU.C) discards the result, so
 * `return 0;` below is a compile-cleanliness addition, not a semantic one). */
dos_int slot_row_draw(struct c470_record *p, dos_int y, dos_int a)
{
    dos_int w;
    dos_char numbuf[6];   /* renamed from the historical `buf` -- collides with the
                            * unrelated global `buf` (DS:BFEE, src/INTRO.C) via game.h */
    dos_char *s;

    if (p->text[0]) {
        /* PORT: historical call passed the record pointer itself
         * (`text_draw_wrapped(43,y,p)`) where text[] is the struct's first
         * member at offset 0 -- passing p->text is the same address. */
        text_draw_wrapped(43, y, p->text);
        s = (p->flags & 16) ? g1650 : g1659;
        text_draw_wrapped(0x92, y, s);
        if (!a) {
            /* p->value is dos_int (struct c470_record +09); the historical
             * cast to (long) sign-extends it, and cc_ultoa's parameter is
             * dos_ulong (Turbo C `ultoa` takes unsigned long) -- the extra
             * (dos_ulong) reinterprets the sign-extended bit pattern
             * exactly as the K&R call (no ultoa prototype in scope) did. */
            ultoa((dos_ulong)(dos_long)p->value, numbuf, 10);
            w = text_line_width(numbuf);
            text_draw_wrapped(0x102 - w, y, numbuf);
        } else {
            /* PORT: sprite_pool_draw_masked's `dst` (src/SPRPOOLD.C, not
             * yet ported) is a historical `char near *` that the callee's
             * own body only ever uses as `gfx_blit_bitmap(dst + k*14, b,
             * ...)` -- and gfx_blit_bitmap's first parameter is ALREADY
             * ported (gfx.h) as a plain `dos_int x` pixel coordinate, not
             * an address (confirmed independently by hudpmsg.c's
             * `gfx_blit_bitmap(6, 0xa2, p)`, a literal small integer that
             * cannot be a meaningful pointer).  So `dst` here is the icon
             * row's starting x-coordinate (0xDD = 221), stepped by 14px
             * per icon, not a real memory address.
             *
             * Investigated whether 0xDD instead names a real DGROUP
             * object (per docs/portable/state-map.md): DS:00DD is the
             * LAST byte of `gbe` (dos_int[16] at DS:00BE, a VGA/EGA
             * color-index lookup table used only by src/VIDEO.C's mode-2
             * gfx_color_select path) -- i.e. the high byte of gbe[15].
             * That object has no conceptual connection to slot-row icon
             * rendering; the byte-offset match is coincidental.  Using
             * `(dos_char *)gbe + 0x1F` here would be WRONG in the
             * portable build: gbe's real address is wherever the linker
             * places it, not DS:00DD, so that expression would not
             * reproduce the value 0xDD at all (whereas the literal-value
             * cast below always does).  sprite_pool_draw_masked's
             * game_funcs.h prototype still spells `dst` as `dos_char *`
             * (unchanged until SPRPOOLD.C itself is ported and, ideally,
             * its signature is fixed to `dos_int dst` the same way
             * gfx_blit_bitmap's was); widened through uintptr_t so the
             * cast is well-defined on a 64-bit pointer target while
             * reproducing the exact 16-bit value 0xDD. */
            sprite_pool_draw_masked((dos_char *)(uintptr_t)(dos_uint)0xdd, y, (dos_uint)p->flags);
        }
    }
    return 0;
}
