/* F_5321 -- the per-unit driver loop.  Plain C; eight of the thirteen
   arguments are addresses of locals, which in compact model are far pointers
   and so compile to push ss / lea ax,[bp-n] / push ax. */
extern void f5382();

void f5321(p1, p2, n, p4, p5, p6)
int p1, p2, n, p4, p5, p6;
{
    int g, f, e, d, c, b, a;   /* TC 2.01 gives the LAST declared local bp-2 */
    g = 0;
    e = 0;
    f = 0;
    b = 0;
    while (g < n)
        f5382(p1, p2, p4, p5, p6, &g, &f, 0, 0, &d, &c, &b, &a);
}
