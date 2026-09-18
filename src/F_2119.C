extern void memmove();
extern unsigned f656c();
extern char far *gc5c6;
extern char g2380[][0x82];

void f2119()
{
    char far *d;
    char far *s;
    int i;
    d = g2380[0];
    s = gc5c6 + 2;
    f656c(0x26);
    for (i = 0; i < 8; ++i) {
        memmove(d, s, 0x82);
        d += 0x82;
        s += 0x84;
        memmove(d, s, 0x62);
        d += 0x62;
        s += 0x64;
        memmove(d, s, 0x92);
        d += 0x92;
        s += 0x94;
    }
}
