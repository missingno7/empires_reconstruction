/* F_8AA2 -- deal two entries into the 4x6 slot grid gC316: for each of two
   rounds take the next free index gC130, pick a random offset into the
   remaining free cells, walk the grid cyclically that many free cells on,
   stamp the index there and give it a random 0..3 attribute when fACE7()
   says so.  Returns whether the grid is now full. */
struct P { unsigned char a, b; };
struct ROW { struct P e[6]; };
extern struct ROW gc316[];
extern int gc130;
extern int rand(void);
extern int face7(void);
extern void f7202(void);

/*@PUB _f8aa2*/
int f8aa2(void)
{
    register int row;
    register int col;
    int slot, r, k, i;   /* TC 2.0 lays locals out in REVERSE declaration
                            order: the LAST declared gets [bp-2] */

    row = 0;
    col = -1;
    for (i = 0; i < 2; i++) {
        if ((slot = gc130++) >= 0xc)
            return -1;
        r = rand() % (0xc - slot);
        for (k = 0; k <= r; k++)
            do {
                if (++col == 3) {
                    col = 0;
                    if (++row == 4)
                        row = 0;
                }
            } while (gc316[row].e[col].a != 0xff);
        gc316[row].e[col].a = slot;
        if (face7())
            gc316[row].e[col].b = rand() % 4;
        else
            gc316[row].e[col].b = 0;
    }
    f7202();
    return gc130 == 0xc;
}
