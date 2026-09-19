/* F_9B68 -- the round opener: clear the three panels, roll a bit out of the
   scenario record, draw the 12 markers, then reset the panels. */
struct evt {
    char pad[0x16];
    char f16;
    char tail[1];
};

extern int f03b4(), face7(), rand(), f656c(), f01ce(), f0355(), f03a8(), f9a0e();
extern struct evt g0dcc[];
extern int gc35b, gc354, gc35d, gc359, gc352;
extern char gc356[];
extern struct { int w0; int w2; } a1271[];

void f9b68(int n)
{
    int a6;
    int a4;
    int i;
    register int x, y;

    x = n;
    for (i = 0; i < 3; i++)
        f03b4(0xf4, i * 0x30 + 0xc8, 0x30, 0x23, 0xf4, i * 0x30 + 0x10);
    gc35b = gc354 = gc35b = 0;
    gc35d = face7() * 0x14 + x;
    while ((g0dcc[gc35d].f16 & (1 << (x = rand() % 8))) == 0)
        ;
    gc359 = x + 1;
    f656c(0x1022);
    for (y = 0; y < 0xc; y++) {
        a6 = a1271[y].w0;
        a4 = a1271[y].w2;
        f01ce(6);
        f0355(a6 - 2, a4 - 2, 0x2e, 0x21);
        f01ce(7);
        f0355(a6 - 1, a4 - 1, 0x2c, 0x1f);
        f01ce(0xf);
        f03a8(a6, a4, 0x2a, 0x1d);
    }
    for (x = 0; x < 9; x++)
        f9a0e(x, x);
    gc356[0] = 9;
    gc356[1] = 0xa;
    gc356[2] = 0xb;
    x = rand() % 3;
    gc352 = x + 9;
    f9a0e(gc352, gc359);
    gc356[x] = gc356[2];
    x = rand() % 2;
    f9a0e(gc356[x], 9);
    f9a0e(gc356[x ^ 1], 0xa);
    for (x = 0; x < 3; x++) {
        f03b4(0xf4, x * 0x30 + 0xc8, 0x30, 0x23, 0xf4, x * 0x30 + 0x158);
        f03b4(0xf4, x * 0x30 + 0x10, 0x30, 0x23, 0xf4, x * 0x30 + 0xc8);
    }
}
