/* src/HINTDLG.C: tutorial hint dialog trigger. */
#include "game.h"

/* ---- F_D4B3 (original code at 0xD4B3) ---- */
/* F_D4B3 -- toggle one bit in the current record's mask and, if it was set,
   look up its string and hand it on.  27-byte records at slot_table. */
void tutorial_hint_dialog_show(dos_int n)
{
    dos_uchar *p;

    if (slot_table[current_slot].option == 0)
        return;
    if (slot_table[current_slot].pending & (1 << n)) {
        slot_table[current_slot].pending ^= (1 << n);
        resource_load_record_alloc(0x101d, &p);
        g2356.text = (dos_char *)(p + ((dos_int *)p)[n] + 2);
        dialog_run(&g2356);
        free(p);
    }
}
