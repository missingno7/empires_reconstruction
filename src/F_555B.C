/* F_555B -- read record 0x33, draw it at the origin and then clear the
   320x200 frame. */
extern int f656c();
extern void f03c9();
extern void f03b4();
extern char far *gc5c6;                 /* DS:C5C6 offset, DS:C5C8 segment */

void f555b()
{
    f656c(0x33);
    f03c9(0, 0, gc5c6);
    f03b4(0, 0, 0x140, 0xc8, 0, 0xc8);
}
