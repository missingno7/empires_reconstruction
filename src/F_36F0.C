/* F_36F0 -- advance every queued 12-byte move record by one step: either
   finish the move (swap the from/to cells, repaint, re-mark the board) or
   redraw the unit at its current cell. */
extern int fcaf1(), f2986(), f03b4();
extern int g96;
extern unsigned char far *g96ea;
extern unsigned char far *gbfbc;

void f36f0(int a)
{
    int n;
    int i;
    unsigned char far *p;
    unsigned char s;
    unsigned char cx;
    unsigned char cy;
    register int u, k;

    g96 = 0x190;
    fcaf1(0xa);
    a++;
    n = *g96ea;
    p = g96ea + 1;
    for (i = 0; i < n; i++, p += 0xc) {
        s = p[4];
        cx = p[0];
        cy = p[1];
        if (p[s + 5] != a) {
            if (s != 0) {
                p[4] = 0;
                f2986(p);
                f03b4(cx * 2, cy + 0xb8, 0x38, 0x10, cx * 2, cy);
            }
        } else {
            p[4] = ++s;
            if (s == 5 || p[s + 5] == 0) {
                p[4] = 0;
                u = p[0] / 4 + (p[1] / 8 - 1) * 0x26 - 1;
                for (k = 0; k < 6; k++, u++)
                    gbfbc[u] = gbfbc[u + 0x26] = 0;
                p[0] = p[2];
                p[1] = p[3];
                p[2] = cx;
                p[3] = cy;
                f03b4(cx * 2, cy + 0x148, 0x38, 0x10, cx * 2, cy);
                f03b4(cx * 2, cy + 0x148, 0x38, 0x10, cx * 2, cy + 0xb8);
                f2986(p);
                cx = p[0];
                cy = p[1];
                f03b4(cx * 2, cy + 0xb8, 0x38, 0x10, cx * 2, cy);
                u = cx / 4 + (cy / 8 - 1) * 0x26 - 1;
                for (k = 0; k < 6; k++, u++)
                    gbfbc[u] = gbfbc[u + 0x26] = 7;
            } else {
                f2986(p);
                f03b4(cx * 2, cy + 0xb8, 0x38, 0x10, cx * 2, cy);
            }
        }
    }
    g96 = 0x9f;
}
