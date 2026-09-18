/* F_2269 -- blit the cursor sprite, and the pending one first.  gc99d2 is a
   far pointer into the 0x2a2-byte sprite table. */
extern int g072c, g072e, g0736, g0738, g073a;
extern char g96ee[];
extern char far *g99d2;
extern int f03cc();

void f2269(void)
{
    if (g072c) {
        f03cc(g0736, g0738, g96ee, 0);
        g072c--;
    }
    f03cc(g0736, g0738, g99d2 + g072e * 0x2a2, g073a);
}
