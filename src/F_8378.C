/* F_8378 -- draw one framed box: three edge blits through the thunk at IP
   03ABh and one fill through 039Fh.  Four parallel int arrays indexed by the
   same parameter, each reached as ONE near DS displacement (rule 2, scalar
   array element); TC 2.0 performs no CSE, so `mov bx,[bp+4] / shl bx,1`
   is recomputed for every one of them (rule 6).  Two register locals take
   SI then DI in declaration order (rule 12) and the two stack locals lie in
   reverse declaration order upward from bp (rule 1). */
extern void f03ab();
extern void f039f();
extern int gc11c[];                     /* DS:C11C */
extern int gc10c[];                     /* DS:C10C */
extern int gc116[];                     /* DS:C116 */
extern int gc124[];                     /* DS:C124 */

void f8378(n)
int n;
{
    int u, v;
    register int x, y;

    x = gc11c[n];
    y = gc10c[n];
    u = gc116[n];
    v = gc124[n];
    f03ab(x + 1, y + 2, u - 2, v - 4);
    f03ab(x + 2, y + 1, u - 4, 1);
    f03ab(x + 2, y + v - 2, u - 4, 1);
    f039f(x, y, u, v);
}
