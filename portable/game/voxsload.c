/* voxsload.c -- ported from src/VOXSLOAD.C (F_DA66: load a slot's two
 * instrument glyphs into their OPL voices via voice_load_instrument,
 * oplreg.c).  No hardware fragments of its own.
 */
#include "game.h"

/* ---- F_DA66 (original code at 0xDA66) ---- */
/* F_DA66 -- draw^H^H^Hload a slot's two glyphs: the pair at record offset
   0x34 with the record itself, and the pair's second half with the
   sub-record at 0x1A.  Word reads through the far pointer are ported via
   dos_rd16 (see oplreg.c's voice_set_frequency for the same substitution)
   instead of the historical `*(int far *)p` cast. */
dos_int voice_slot_load_pair(dos_int i, dos_char *a)
{
    dos_char *q;
    dos_char *p;
    dos_int s, d;

    p = a + 0x34;
    s = dos_i16(dos_rd16((const uint8_t *)p));
    p += 2;
    d = dos_i16(dos_rd16((const uint8_t *)p));
    q = a + 0x1a;
    voice_load_instrument(ui_panel_glyph_records[i].b0, a, s);
    voice_load_instrument(ui_panel_glyph_records[i].b1, q, d);

    /* PORT: K&R implicit-int return (src/VOXSLOAD.C never has a `return`
       statement; Turbo C let this fall through with whatever was left in
       AX).  Its sole caller, src/RECTTAB.C:145, discards the result
       (`voice_slot_load_pair(i, ...)` used as a bare statement), so the
       return value is dead in every real call site; return 0. */
    return 0;
}
