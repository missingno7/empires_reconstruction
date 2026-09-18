/* F_8434 -- one blit through the runtime-generated thunk at IP 03AEh, six
   DGROUP words wide.  Two adjacent words pushed segment-then-offset and two
   plain ints are the same six pushes, so they are spelled as the six ints
   the extent literally pushes. */
extern void f03ae();
extern unsigned gc10a;                  /* DS:C10A */
extern unsigned gc106;                  /* DS:C106 */
extern unsigned gc108;                  /* DS:C108 */
extern unsigned gc11a;                  /* DS:C11A */
extern unsigned gc5ca;                  /* DS:C5CA */
extern unsigned gc5cc;                  /* DS:C5CC */

void f8434()
{
    f03ae(gc10a, gc106, gc108, gc11a, gc5ca, gc5cc);
}
