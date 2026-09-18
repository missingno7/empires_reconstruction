/* F_684A -- read one indexed record, allocate a far block for it and copy the
   staged buffer in; on an allocation failure, tear the screen down, print the
   message at DS:0B42 and exit.  The far pointer OUT parameter is compared
   against zero the way TC 2.0 compares a far pointer: OR the two halves. */
extern int f656c();
extern char far *farmalloc();
extern void f49e3();
extern void f034f();
extern void f4f63();
extern void exit();
extern void movmem();
extern char far *gc5c6;                 /* DS:C5C6 offset, DS:C5C8 segment */
extern char gb42[];                     /* DS:0B42, the message */

void f684a(a, pp)
int a;
char far * far *pp;
{
    register int n;

    n = f656c(a);
    *pp = farmalloc((long)n);
    if (*pp == 0) {
        f49e3();
        f034f();
        f4f63(gb42);
        exit(a);
    }
    movmem(gc5c6, *pp, n);
}
