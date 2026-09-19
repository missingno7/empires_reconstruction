/* F_462E -- paint the title/menu backdrop for the current mode.  si is the
   mode, di the row offset chosen by the two-term disjunction at 4698. */
extern int g9ade;
extern int gbfba;
extern char far *gbfbc;
extern char far *gc5c6;
extern unsigned char g4374[];
extern char g0b3ae[];
extern int f1d47(), f1ec0(), f656c(), face7(), movmem(), setmem(), f4517();

struct big { char pad[0x3e8]; };
extern struct big g43b4[];

void f462e(void)
{
    register int m, d;

    f1d47();
    if (f1ec0(g9ade))
        m = 0x14;
    else
        m = g9ade / 2;
    if (m == 0x15)
        f656c(0x42);
    else
        f656c(m + 0x1000);
    if (m == 0x14) {
        movmem(gc5c6, g4374, 0x2750);
        setmem(g0b3ae, 0xbb8, 0);
    } else {
        if (face7() == 0 || m > 0x14)
            d = 0;
        else
            d = 0x330c;
        movmem(gc5c6 + d + 2, g4374, 0x2750);
        movmem(gc5c6 + d + 0x2754, g0b3ae, 0xbb8);
    }
    if (m < 0x14)
        f4517(g4374[0] & 0x7f);
    gbfbc = (char far *) &g43b4[gbfba = 0];
}
