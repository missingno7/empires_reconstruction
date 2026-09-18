/* F_21DB -- read record 4 into the staging block, allocate 15,502 bytes and
   copy 23 stripes of 674 bytes out of the staging block (676-byte pitch, two
   header bytes skipped) into it, then one more stripe into the DS:96EE
   buffer.  No frame: no parameter and no local (rule 11); the loop counter is
   the one register variable. */
extern int f656c();
extern char far *malloc();
extern void movmem();
extern char far *g99d2;                 /* DS:99D2 offset, DS:99D4 segment */
extern char far *gc5c6;                 /* DS:C5C6 offset, DS:C5C8 segment */
extern unsigned char g96ee[];           /* DS:96EE */

void f21db()
{
    register int i;

    f656c(4);
    g99d2 = malloc(0x3c8e);
    for (i = 0; i < 23; i++)
        movmem(gc5c6 + i * 676 + 2, g99d2 + i * 674, 674);
    movmem(gc5c6 + 0x3cbe, g96ee, 674);
}
