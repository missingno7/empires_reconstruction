/* F_D4B3 -- toggle one bit in the current record's mask and, if it was set,
   look up its string and hand it on.  27-byte records at slot_table. */
#include "C470.H"
#include "DIALOG.H"

extern void resource_load_record_alloc();
extern void farfree();
extern char far *g235d;
extern struct dialog g2356;

void fd4b3(int n)
{
    char far *p;

    if (slot_table[current_slot].option == 0)
        return;
    if (slot_table[current_slot].pending & (1 << n)) {
        slot_table[current_slot].pending ^= (1 << n);
        resource_load_record_alloc(0x101d, &p);
        g235d = p + ((int far *) p)[n] + 2;
        dialog_run(&g2356);
        farfree(p);
    }
}
