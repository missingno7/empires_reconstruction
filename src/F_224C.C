/* F_224C -- allocate the 0x55F0-byte arena, keep the far pointer at DS:99D6
   and run the three initialisers over it.  `xor dx,dx` before the push makes
   the size one 32-bit argument, not two ints, and the result comes back in
   DX:AX -- farmalloc, CC.LIB at IP 0E9B4h (substrate/LIB_FMALLOC.json). */
extern char far *farmalloc();
extern void f200f();
extern void f2119();
extern void f6fca();
extern char far *g99d6;                 /* DS:99D6 offset, DS:99D8 segment */

void f224c()
{
    g99d6 = farmalloc(0x55f0L);
    f200f();
    f2119();
    f6fca();
}
