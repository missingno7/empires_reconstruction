/* F_50C1 -- read the BIOS equipment word and store (bits 6-5) + 1 at DS:BFCC.
   The int 11h is inline asm and the arithmetic is C through the `_AX`
   pseudo-variable; no frame, because there is no parameter and no local
   (rule 11), and the body names neither SI nor DI (rule 14). */
extern unsigned char gbfcc;             /* DS:BFCC */

void f50c1()
{
    asm xor ax,ax
    asm int 11h
    gbfcc = ((_AX & 0xc0) >> 6) + 1;
}
