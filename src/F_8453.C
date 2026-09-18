/* F_8453 -- the pair of blits through the runtime-generated thunks at IP
   03B1h and 039Fh. */
extern void f03b1();
extern void f039f();
extern unsigned gc10a;                  /* DS:C10A */
extern unsigned gc106;                  /* DS:C106 */
extern unsigned gc108;                  /* DS:C108 */
extern unsigned gc11a;                  /* DS:C11A */
extern unsigned gc5ca;                  /* DS:C5CA */
extern unsigned gc5cc;                  /* DS:C5CC */

void f8453()
{
    f03b1(gc10a, gc106, gc5ca, gc5cc);
    f039f(gc10a, gc106, gc108, gc11a);
}
