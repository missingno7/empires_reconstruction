/* F_68AA -- read one indexed record through f656c and copy the staged buffer
   the far pointer at DS:C5C6 names into the caller's buffer.  The pointer is
   a far POINTER global: it pushes segment word then offset word
   (tc20-codegen rule 5), which an array name would not have done. */
extern void movmem();
extern unsigned f656c();
extern char far *gc5c6;                 /* DS:C5C6 offset, DS:C5C8 segment */

void f68aa(a, q)
unsigned a;
char far *q;
{
    register unsigned n;

    n = f656c(a);
    movmem(gc5c6, q, n);
}
