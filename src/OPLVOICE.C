/* src/OPLVOICE.C: OPL voice bank control.
   One translation unit; the sections below were the separate member
   sources of grouped module C_DB60_DDC7 and keep their original ids. */

#include "G2FD2.H"

extern char en[9];                      /* DS:C6AB */
/*@SYM _en=0xC6AB kind=g key=storage_objects/REGION_2C1DB.phys*/
extern char off[9];                     /* DS:CA62 */
/*@SYM _off=0xCA62 kind=g key=storage_objects/M_2C592.phys*/
extern void voice_key_off();
extern void voice_set_frequency();
extern void voice_update_tone();
extern void voice_set_field();
extern char gc6b6[];                    /* DS:C6B6, byte per voice */
extern unsigned char voice_level_table[];           /* DS:CA50 */

/* ---- F_DB60 (original code at 0xDB60) ---- */
/* F_DB60 -- retune the enabled voices of one bank.  Plain C; the register
   assignment (base -> si, bank -> di) is fixed by the order the two register
   parameters are DECLARED, not by the order they appear in the list. */
void voice_bank_retune_on(bank, base)
register int base;
register unsigned bank;
{
    if (bank > 2) return;
    if (bank == 0) {
        if (en[0]) { voice_key_off(0); voice_set_frequency(0, off[0] + base, 1); }
        if (en[1]) { voice_key_off(1); voice_set_frequency(1, off[1] + base, 1); }
        if (en[2]) { voice_key_off(2); voice_set_frequency(2, off[2] + base, 1); }
    }
    if (bank == 1) {
        if (en[3]) { voice_key_off(3); voice_set_frequency(3, off[3] + base, 1); }
        if (en[4]) { voice_key_off(4); voice_set_frequency(4, off[4] + base, 1); }
        if (en[5]) { voice_key_off(5); voice_set_frequency(5, off[5] + base, 1); }
    }
    if (bank == 2) {
        if (en[6]) { voice_key_off(6); voice_set_frequency(6, off[6] + base, 1); }
        if (en[7]) { voice_key_off(7); voice_set_frequency(7, off[7] + base, 1); }
        if (en[8]) { voice_key_off(8); voice_set_frequency(8, off[8] + base, 1); }
    }
}


/* ---- F_DCC7 (original code at 0xDCC7) ---- */
/* F_DCC7 -- fire the enabled voices of one bank.  Plain C. */
void voice_bank_update(bank)
int bank;
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
/* F_DD72 -- retune one voice: two voice_set_field calls for the two bytes of the
   2-byte record at DS:2FD2, then reprogram the voice through voice_set_frequency. */
void voice_update_tone(v)
register int v;
{
    voice_set_field(ui_panel_glyph_records[v].b0, 7, 0xa);
    voice_set_field(ui_panel_glyph_records[v].b1, 7, 0xa);
    voice_set_frequency(v, gc6b6[v], 0);
}


/* ---- F_DDC7 (original code at 0xDDC7) ---- */
/* F_DDC7 -- fill the 18 bytes at DS:CA50 with 0x7F.  The counter is the one
   register variable (SI), the index is signed (`jl`), and the pre-test loop
   opens with the jump to the test that TC 2.0 emits for `for`. */
void voice_level_table_reset()
{
    register int i;

    for (i = 0; i < 18; i++)
        voice_level_table[i] = 0x7f;
}
