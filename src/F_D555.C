/* F_D555 -- allocate the 0x620-byte staging block, read record 0x41 into a
   second far block through F_684A, publish both addresses into the DS:175E
   table and repaint.  The four publications are FOUR WORD copies, not two far
   pointer copies: a far pointer assignment would have opened `les`, and the
   image opens `mov ax,[C5E0]`. */
extern char far *farmalloc();
extern void f684a();
extern void fc77a();
extern char far *gc5da;                 /* DS:C5DA offset, DS:C5DC segment */
extern unsigned gc5dc;                  /* DS:C5DC */
extern unsigned gc5de;                  /* DS:C5DE */
extern unsigned gc5e0;                  /* DS:C5E0 */
extern unsigned g175e, g1760, g1762, g1764;

void fd555()
{
    gc5da = farmalloc(0x620L);
    f684a(0x41, &gc5de);
    g1760 = gc5e0;
    g175e = gc5de;
    g1764 = gc5dc;
    g1762 = (unsigned) gc5da;
    fc77a();
}
