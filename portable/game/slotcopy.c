/* slotcopy.c -- portable port of src/SLOTCOPY.C: save-slot transfer
 * copies.
 * One translation unit; the sections below were the separate member
 * sources of grouped module RELOC_F_AD25_F_ADCF and keep their original
 * ids.
 */
#include "game.h"

/* ---- F_AD25 (original code at 0xAD25) ---- */
void slot_archive_and_delete(void)
{
    dos_int i;

    for (i = 0; i < 10; i++) {
        if (!slot_transfer_table[i].text[0]) {
            slot_transfer_table[i] = slot_table[current_slot];
            break;
        }
    }
    if (i == 10) {
        for (i = 1; i < 10; i++)
            slot_transfer_table[i - 1] = slot_transfer_table[i];
        slot_transfer_table[9] = slot_table[current_slot];
    }
    slot_delete(current_slot);
    slot_table_save();
}

/* ---- F_ADCF (original code at 0xADCF) ---- */
dos_int slot_reset_for_new_game(void)
{
    dos_int i;

    for (i = 0; i < 10; i++) {
        if (!slot_transfer_table[i].text[0]) {
            slot_transfer_table[i] = slot_table[current_slot];
            break;
        }
    }
    if (i == 10) {
        for (i = 1; i < 10; i++)
            slot_transfer_table[i - 1] = slot_transfer_table[i];
        slot_transfer_table[9] = slot_table[current_slot];
    }
    slot_table[current_slot].flags = 32;
    slot_table[current_slot].state = 4;
    slot_table[current_slot].value = 1;
    slot_table[current_slot].option = 0;
    slot_table[current_slot].resume_round = 0;
    slot_table[current_slot].round_progress[0] = 0;
    slot_table[current_slot].round_progress[1] = 0;
    slot_table[current_slot].round_progress[2] = 0;
    slot_table[current_slot].round_progress[3] = 0;
    slot_table[current_slot].byte26 = 0;
    slot_table_save();
    return 0;   /* PORT: value unused (K&R implicit int) */
}
