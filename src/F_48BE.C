/* F_48BE -- patch the blitter at IP 0x039C in place with the variant record
   for the current display mode.  The destination is a far pointer to CODE,
   widened from the near function with a cast, which is why it pushes
   `cs` and not a segment fixup. */
extern int f656c();
extern void memmove();
extern void f039c();
extern char gbfcd;                      /* DS:BFCD, the display mode */
extern char far *gc5c6;                 /* DS:C5C6 offset, DS:C5C8 segment */

void f48be()
{
    register int n;

    if (gbfcd == 5) {
        n = f656c(2);
        memmove((char far *) f039c, gc5c6, n);
    } else if (gbfcd == 2) {
        n = f656c(3);
        memmove((char far *) f039c, gc5c6, n);
    }
}
