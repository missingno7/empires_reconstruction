/* F_5E98 -- redraw the 0x18 sprite slots, then append four bytes to the
   command stream at g40c4.  Plain C. */
extern int f020f(), f03d5(), f03d2(), f01ce();
extern int gc050[], gc080[];
extern int g40c8;
extern int gc046, gc048;
extern char gc04a, gc04c;
extern char far *g40c4;

void f5e98(void)
{
    int w4, saved;
    register int i, p;

    saved = f020f();
    for (i = 0; i < 0x18; i++) {
        if ((p = gc050[i]) != 0) {
            g40c8 = f03d5(p, (w4 = gc080[i]) + 0xb8);
            f03d2(p, w4);
        }
    }
    if (gc048 != 0) {
        *g40c4++ = gc046 >> 1;
        *g40c4++ = gc04a;
        *g40c4++ = (gc048 - gc046 + 4) >> 1;
        *g40c4++ = gc04c - gc04a + 1;
    }
    f01ce(saved);
}
