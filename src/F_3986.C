extern int gbfba, g73e, g96, g736, g738;
extern void f4eeb(int n);
extern void f9f40(int x, int y, int w, int h, int x2, int y2);
struct REC1B {
    char pad0[9];
    int f9;
    char pad1[10];
    char f15;
    char pad2[5];
};
extern struct REC1B gc470[];
extern int g13ed;
extern void f7343(int n);
extern int fa768(void);
extern char g8bfe;
extern void longjmp(char far *s, int n);

void f3986(void)
{
    int si, di, w, h;

    g73e = gbfba;
    g96 = 0x190;
    f4eeb(0xb8);
    g96 = 0x9f;

    if (g736 < 8) {
        si = 8;
        w = 0x28;
    } else if (g736 + 0x27 > 0x137) {
        si = g736;
        w = 0x138 - g736;
    } else {
        si = g736;
        w = 0x28;
    }

    if (g738 < 0x10) {
        di = 0x10;
        h = 0x28;
    } else if (g738 + 0x27 > 0x9f) {
        di = g738;
        h = 0xa0 - g738;
    } else {
        di = g738;
        h = 0x28;
    }

    f9f40(si, di + 0xb8, w, h, si, di);

    gc470[g13ed].f9++;
    f7343(gc470[g13ed].f15 = 4);
    if (fa768())
        longjmp((char far *) &g8bfe, 2);
}
