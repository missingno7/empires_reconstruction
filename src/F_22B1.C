/* F_22B1 -- clamp the view window to the board and blit it.  Plain C.
   si/di are the two register variables, [bp-4] and [bp-2] the two ints. */
extern int g736, g738;
extern int f03b4();

void f22b1(void)
{
    int w4, w2;
    register int x, y;

    if (g736 < 8) {
        x = 8;
        w4 = 0x28;
    } else if (g736 + 0x27 > 0x137) {
        x = g736;
        w4 = 0x138 - g736;
    } else {
        x = g736;
        w4 = 0x28;
    }
    if (g738 < 0x10) {
        y = 0x10;
        w2 = 0x28;
    } else if (g738 + 0x27 > 0x9f) {
        y = g738;
        w2 = 0xa0 - g738;
    } else {
        y = g738;
        w2 = 0x28;
    }
    f03b4(x, y + 0xb8, w4, w2, x, y);
}
