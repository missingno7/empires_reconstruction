/* F_9F40 -- 0x10-step animation loop; the blit call is scaled by the video
   mode in gbfcd.  q is the register copy of the third parameter. */
extern int f6c57(), f9ec3(), f039f(), f6c6f();
extern char gbfcd;

void f9f40(int a, int b, int c, int d, int e, int f)
{
    register int i, q;

    q = c;
    for (i = 0; i < 0x10; i++) {
        f6c57(9);
        if (gbfcd == 5)
            f9ec3(a, b, q, d, e, f, i, 0x140);
        else if (gbfcd == 2)
            f9ec3(a / 4, b, q / 4, d, e / 4, f, i, 0x50);
        else
            f9ec3(a / 2, b, q / 2, d, e / 2, f, i, 0xa0);
        f039f(e, f, q, d);
        f6c6f();
    }
}
