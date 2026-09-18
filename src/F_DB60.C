/* F_DB60 -- retune the enabled voices of one bank.  Plain C; the register
   assignment (base -> si, bank -> di) is fixed by the order the two register
   parameters are DECLARED, not by the order they appear in the list. */
extern char en[9];                      /* DS:C6AB */
/*@SYM _en=0xC6AB kind=g key=storage_objects/REGION_2C1DB.phys*/
extern char off[9];                     /* DS:CA62 */
/*@SYM _off=0xCA62 kind=g key=storage_objects/M_2C592.phys*/
extern void fe52a();
extern void fe48a();

void fdb60(bank, base)
register int base;
register unsigned bank;
{
    if (bank > 2) return;
    if (bank == 0) {
        if (en[0]) { fe52a(0); fe48a(0, off[0] + base, 1); }
        if (en[1]) { fe52a(1); fe48a(1, off[1] + base, 1); }
        if (en[2]) { fe52a(2); fe48a(2, off[2] + base, 1); }
    }
    if (bank == 1) {
        if (en[3]) { fe52a(3); fe48a(3, off[3] + base, 1); }
        if (en[4]) { fe52a(4); fe48a(4, off[4] + base, 1); }
        if (en[5]) { fe52a(5); fe48a(5, off[5] + base, 1); }
    }
    if (bank == 2) {
        if (en[6]) { fe52a(6); fe48a(6, off[6] + base, 1); }
        if (en[7]) { fe52a(7); fe48a(7, off[7] + base, 1); }
        if (en[8]) { fe52a(8); fe48a(8, off[8] + base, 1); }
    }
}
