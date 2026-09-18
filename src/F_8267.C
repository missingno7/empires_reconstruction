/* F_8267 -- draw one box's border: eight edge/corner blits through the
   thunks at IP 03A2h and 03A5h, then the caption.  The same four parallel
   int arrays F_8378 uses, each reached as ONE near DS displacement and
   recomputed for every access (rules 2 and 6).  SI and DI go by DECLARATION
   order, not by first use (rule 22): here the DS:C10C value is declared
   first and takes SI, and the DS:C11C value takes DI -- the opposite
   assignment to F_8378, from the opposite declaration order.  The caption's
   third argument is a NEAR pointer widened by a cast, so the segment is
   pushed as plain `ds` (rule 20's data form). */
extern void f03a2();
extern void f03a5();
extern void f6d3c();
extern int gc11c[];                     /* DS:C11C */
extern int gc10c[];                     /* DS:C10C */
extern int gc116[];                     /* DS:C116 */
extern int gc124[];                     /* DS:C124 */
extern int gc12c[];                     /* DS:C12C */
extern char near *gc112[];              /* DS:C112 */

void f8267(n)
int n;
{
    int u, v;
    register int y, x;

    x = gc11c[n];
    y = gc10c[n];
    u = gc116[n];
    v = gc124[n];
    f03a2(x + 1, y, u - 2);
    f03a2(x + 1, y + v - 1, u - 2);
    f03a5(x, y + 1, v - 2);
    f03a5(x + u - 1, y + 1, v - 2);
    f03a2(x + 1, y + 1, 1);
    f03a2(x + 1, y + v - 2, 1);
    f03a2(x + u - 2, y + 1, 1);
    f03a2(x + u - 2, y + v - 2, 1);
    f6d3c(gc12c[n], (v - 10) / 2 + y + 1, (char far *)gc112[n]);
}
