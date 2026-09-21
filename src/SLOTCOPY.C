/* src/SLOTCOPY.C: Save-slot transfer copies.
   One translation unit; the sections below were the separate member
   sources of grouped module RELOC_F_AD25_F_ADCF and keep their original ids. */

#include "C470.H"
extern int current_slot;
extern void slot_delete(int), slot_table_save();

/* ---- F_AD25 (original code at 0xAD25) ---- */
void fad25()
{
    register int i;

    for (i = 0; i < 10; i++) {
        if (!gc360[i].text[0]) {
            gc360[i] = slot_table[current_slot];
            break;
        }
    }
    if (i == 10) {
        for (i = 1; i < 10; i++)
            gc360[i - 1] = gc360[i];
        gc360[9] = slot_table[current_slot];
    }
    slot_delete(current_slot);
    slot_table_save();
}


/* ---- F_ADCF (original code at 0xADCF) ---- */
fadcf()
{
    register int i;

    for (i = 0; i < 10; i++) {
        if (!gc360[i].text[0]) {
            gc360[i] = slot_table[current_slot];
            break;
        }
    }
    if (i == 10) {
        for (i = 1; i < 10; i++)
            gc360[i - 1] = gc360[i];
        gc360[9] = slot_table[current_slot];
    }
    slot_table[current_slot].flags = 32;
    slot_table[current_slot].state = 4;
    slot_table[current_slot].value = 1;
    slot_table[current_slot].option = 0;
    slot_table[current_slot].byte12 = 0;
    slot_table[current_slot].sub[0] = 0;
    slot_table[current_slot].sub[1] = 0;
    slot_table[current_slot].sub[2] = 0;
    slot_table[current_slot].sub[3] = 0;
    slot_table[current_slot].byte26 = 0;
    slot_table_save();
}
