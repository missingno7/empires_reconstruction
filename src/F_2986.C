extern void f03cc();
extern int g00bc;
extern char gb1cc[][1], g893c[][0x62];

void f2986(p)
unsigned char far *p;
{
    int saved;
    int n;
    int off;
    register int di;
    register int i;

    saved = g00bc;
    di = p[0] * 2;
    n = p[1];
    f03cc(di, n + 0xb8, gb1cc[0], 0);
    g00bc = 0;
    i = p[4] + 5;
    while (i < 10) {
        if (p[i] == 0)
            break;
        off = (i - 5) * 10 + di + 6;
        f03cc(off, n + 0xb9, g893c[p[i] - 1], 0);
        ++i;
    }
    g00bc = saved;
}
