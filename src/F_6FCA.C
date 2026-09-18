/* F_6FCA -- load record 0x3F into the far block whose pointer lives at
   DS:C0EE.  The `push ds / mov ax,0C0EEh / push ax` pair is the ADDRESS of
   that pointer, F_684A's `char far * far *` OUT parameter. */
extern void f684a();
extern char far *gc0ee;                 /* DS:C0EE offset, DS:C0F0 segment */

void f6fca()
{
    f684a(0x3f, &gc0ee);
}
