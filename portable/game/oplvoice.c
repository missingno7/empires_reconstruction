/* oplvoice.c -- ported from src/OPLVOICE.C (OPL voice bank control).
 *
 * No hardware fragments of its own; every call in this file goes through
 * already-ported oplreg.c entry points (voice_key_off, voice_set_frequency,
 * voice_set_field), which themselves call opl_write().
 */
#include "game.h"

/* ---- F_DB60 (original code at 0xDB60) ---- */
/* F_DB60 -- retune the enabled voices of one bank.  Plain C; the register
   assignment (base -> si, bank -> di) is fixed by the order the two register
   parameters are DECLARED, not by the order they appear in the list. */
void voice_bank_retune_on(dos_uint bank, dos_int base)
{
    if (bank > 2) return;
    if (bank == 0) {
        if (en[0]) { voice_key_off(0); voice_set_frequency(0, gca62[0] + base, 1); }
        if (en[1]) { voice_key_off(1); voice_set_frequency(1, gca62[1] + base, 1); }
        if (en[2]) { voice_key_off(2); voice_set_frequency(2, gca62[2] + base, 1); }
    }
    if (bank == 1) {
        if (en[3]) { voice_key_off(3); voice_set_frequency(3, gca62[3] + base, 1); }
        if (en[4]) { voice_key_off(4); voice_set_frequency(4, gca62[4] + base, 1); }
        if (en[5]) { voice_key_off(5); voice_set_frequency(5, gca62[5] + base, 1); }
    }
    if (bank == 2) {
        if (en[6]) { voice_key_off(6); voice_set_frequency(6, gca62[6] + base, 1); }
        if (en[7]) { voice_key_off(7); voice_set_frequency(7, gca62[7] + base, 1); }
        if (en[8]) { voice_key_off(8); voice_set_frequency(8, gca62[8] + base, 1); }
    }
}


/* ---- F_DCC7 (original code at 0xDCC7) ---- */
/* F_DCC7 -- fire the enabled voices of one bank.  Plain C. */
void voice_bank_update(dos_int bank)
{
    if (bank > 2) return;
    if (bank == 0) {
        if (en[0]) voice_update_tone(0);
        if (en[1]) voice_update_tone(1);
        if (en[2]) voice_update_tone(2);
        return;
    }
    if (bank == 1) {
        if (en[3]) voice_update_tone(3);
        if (en[4]) voice_update_tone(4);
        if (en[5]) voice_update_tone(5);
        return;
    }
    if (bank == 2) {
        if (en[6]) voice_update_tone(6);
        if (en[7]) voice_update_tone(7);
        if (en[8]) voice_update_tone(8);
    }
}


/* ---- F_DD72 (original code at 0xDD72) ---- */
/* F_DD72 -- retune one voice: two voice_set_field calls for the two bytes of
   the 2-byte record at DS:2FD2, then reprogram the voice through
   voice_set_frequency. */
void voice_update_tone(dos_int v)
{
    voice_set_field(ui_panel_glyph_records[v].b0, 7, 0xa);
    voice_set_field(ui_panel_glyph_records[v].b1, 7, 0xa);
    voice_set_frequency(v, gc6b6[v], 0);
}


/* ---- F_DDC7 (original code at 0xDDC7) ---- */
/* F_DDC7 -- fill the 18 bytes at DS:CA50 with 0x7F.  The counter is the one
   register variable (SI), the index is signed (`jl`), and the pre-test loop
   opens with the jump to the test that TC 2.0 emits for `for`.
   int-semantics-inventory.md fact 15 (src/OPLVOICE.C:84-90): i stays
   dos_int (no helper needed, loop bound 18 is far below any 16/32-bit
   behavioral difference). */
void voice_level_table_reset(void)
{
    dos_int i;

    for (i = 0; i < 18; i++)
        voice_level_table[i] = 0x7f;
}
