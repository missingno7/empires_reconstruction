/*@FLAGS -B */
/* F_2AE2 -- the board redraw.  Plain C.  The module is on the TASM path:
   the two  83 E3 3F  (and bx,3fh) at 2C31 and 2D78 are the short AND form
   TASM picks and TCC's own writer does not. */
/*@PUB _f2ae2*/
extern int f03c9(), f03cc(), f03b4(), f1ec0(), f656c(), f2986();
extern int f_28ac(), f6036(), f_d61c(), fd818();
extern void fd825(char, int, int, int, int);
extern unsigned char near *f2a70();

extern unsigned char far *gbfbc, far *gbfc4;
extern unsigned char far *g96e6, far *g96ea, far *g40d0;
extern char far *gbfc0, far *gc5c6;
extern int g96, g94, g722, g73c, gbfba, g9ade, gb07a;
extern int w8bea[], w8bf4[];
extern unsigned char b4377[], b437a[], b4380[], b4386[];
extern char s8c12[], s79bf[], s7400[], s735e[];
extern char s9b6e[], s9ae0[], s9a5c[], s99da[], s9c50[], s6e88[];
extern char far *a72b2[];
extern char a74a2[][0xbb], a6f2a[][0xe2], a893c[][0x62];

void f2ae2()
{
    unsigned char c;
    unsigned char far *p;
    unsigned char far *q;
    int w8;
    int n;
    int w4;
    int w2;
    register int i, j;

    if (g9ade == 0x2a)
        return;
    gbfc0 = (char far *) (gbfbc + 0x2ca);
    f03c9(8, 0xc8, s8c12);
    f03c9(0x54, 0xc8, s8c12);
    f03c9(0xa0, 0xc8, s8c12);
    f03c9(0xec, 0xc8, s8c12);
    f03c9(8, 0x110, s8c12);
    f03c9(0x54, 0x110, s8c12);
    f03c9(0xa0, 0x110, s8c12);
    f03c9(0xec, 0x110, s8c12);
    g96 = 0x190;
    if (f1ec0(g9ade) != 0) {
        f656c(g73c + 0x101e);
        f03cc(8, 0xc8, gc5c6, 0);
    }
    n = *(q = f2a70());
    p = q + 1;
    for (i = 0; i < n; i++, p += 3)
        if (p[2] >= 0x80) {
            j = p[0] * 2;
            w8 = p[1];
            f03cc(j, w8 + 0xb8, a72b2[p[2] & 0x3f], p[2] & 0x40);
        }
    gbfc4 = p;
    g94 = 0xc8;
    i = 0;
    w8 = i;
    for (; i < 0x12; i++)
        for (j = 0; j < 0x26; j++) {
            if ((c = gbfbc[w8]) & 7) {
                c = (c & 7) - 1;
                if (c < 6)
                    f03cc(j * 8 + 4, i * 8 + 0xc4, a74a2[c], 0);
            } else if (c & 0x80) {
                c = (c & 0x70) >> 4;
                if (c != 0)
                    f03cc(j * 8 + 8, i * 8 + 0xc8, a6f2a[c - 1], 0);
            }
            w8++;
        }
    g94 = 0x10;
    p = q + 1;
    for (i = 0; i < n; i++, p += 3)
        if (p[2] < 0x80) {
            j = p[0] * 2;
            w8 = p[1];
            f03cc(j, w8 + 0xb8, a72b2[p[2] & 0x3f], p[2] & 0x40);
        }
    if (gb07a != 0) {
        if (b4377[0] == gbfba) {
            g722 = 1;
            w8bea[0] = b4377[1];
            w8bea[0] <<= 1;
            w8bf4[0] = b4377[2];
        } else {
            g722 = 0;
        }
    }
    for (i = 0; i < g722; i++)
        f03c9(w8bea[i], w8bf4[i] + 0xb8, s79bf);
    if (*gbfc0 != 0)
        f_d61c();
    f03b4(0, 0xc8, 0x140, 0x90, 0, 0x158);
    g96 = 0x190;
    f_28ac();
    fd818();
    for (i = 0; i < 6; i++)
        if (gbfba + 1 == b437a[i]) {
            f03cc((w8 = b4380[i]) * 2, (j = b4386[i]) + 0xb8, s7400, 0);
            fd825(i + 1, w8, j, 8, 0x10);
        }
    if (gbfbc[0x3e7] == gbfba + 1) {
        f03cc((w8 = gbfbc[0x3e5]) * 2, (j = gbfbc[0x3e6]) + 0xb8, s735e, 0);
        fd825(7, w8, j, 8, 0x10);
    }
    p = (unsigned char far *) (gbfc0 + *gbfc0 * 4 + 1);
    n = *p;
    i = 0;
    p++;
    for (; i < n; i++, p += p[0]) {
        j = p[2];
        w8 = p[3];
        if ((w4 = p[1]) > 2) {
            fd825(i + 8, j, w8, 4, 8);
        } else if (w4 == 0) {
            if (p[4] != 0)
                f03cc(j * 2, w8 + 0xb8, s9b6e, 0);
            else
                f03cc(j * 2, w8 + 0xb8, s9ae0, 0);
            fd825(i + 8, j + 6, w8 + 3, 2, 6);
        } else if (w4 == 1) {
            if (p[4] != 0)
                f03cc(j * 2, w8 + 0xb8, s9a5c, 0);
            else
                f03cc(j * 2, w8 + 0xb8, s99da, 0);
            fd825(i + 8, j + 3, w8, 7, 7);
        } else {
            f03cc(j * 2, w8 + 0xb8, s9c50, 0);
            fd825(i + 8, j, w8, 8, 0x10);
        }
    }
    n = *(g96e6 = p);
    i = 0;
    p++;
    for (; i < n; i++, p += 3) {
        j = p[0] * 2;
        w8 = p[1];
        f03cc(j, w8 + 0xb8, s6e88, 0);
        f03cc(j + 4, w8 + 0xb8, a893c[p[2]], 0);
        fd825(p[2] + 0x20, p[0] + 1, w8 + 4, 6, 0xa);
    }
    n = *(g96ea = p);
    i = 0;
    p++;
    for (; i < n; i++, p += 0xc) {
        f2986(p);
        w2 = p[0] / 4 + (p[1] / 8 - 1) * 0x26 - 1;
        for (w8 = 0; w8 < 6; w8++, w2++)
            gbfbc[w2] = gbfbc[w2 + 0x26] = 7;
    }
    if (*(g40d0 = p) != 0)
        f6036();
    g96 = 0x9f;
}
