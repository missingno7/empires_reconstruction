/* F_DCC7 -- fire the enabled voices of one bank.  Plain C. */
extern char en[9];                      /* DS:C6AB */
/*@SYM _en=0xC6AB kind=g key=storage_objects/REGION_2C1DB.phys*/
extern void fdd72();

void fdcc7(bank)
int bank;
{
    if (bank > 2) return;
    if (bank == 0) {
        if (en[0]) fdd72(0);
        if (en[1]) fdd72(1);
        if (en[2]) fdd72(2);
        return;
    }
    if (bank == 1) {
        if (en[3]) fdd72(3);
        if (en[4]) fdd72(4);
        if (en[5]) fdd72(5);
        return;
    }
    if (bank == 2) {
        if (en[6]) fdd72(6);
        if (en[7]) fdd72(7);
        if (en[8]) fdd72(8);
    }
}
