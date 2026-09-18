/* F_D344 -- carve three far blocks out of one 64,128-byte allocation, each at
   a paragraph boundary derived from the first segment plus one.  The size is
   an unsigned long CONSTANT (`xor dx,dx`, not `cwd` -- the negative of rule
   16's signed-int case). */
#define MK_FP(seg, ofs) ((void far *) (((unsigned long) (seg) << 16) | (unsigned) (ofs)))
#define FP_SEG(fp) ((unsigned) ((unsigned long) (void far *) (fp) >> 16))

extern char far *farmalloc();
extern char far *gc5ca;                 /* DS:C5CA offset, DS:C5CC segment */
extern char far *gc5c6;                 /* DS:C5C6 offset, DS:C5C8 segment */
extern char far *gc5be;                 /* DS:C5BE offset, DS:C5C0 segment */

void fd344()
{
    register unsigned s;

    gc5ca = farmalloc(0xfa80L);
    s = FP_SEG(gc5ca) + 1;
    gc5ca = MK_FP(s, 0x0e);
    gc5c6 = MK_FP(s + 1, 0);
    gc5be = MK_FP(s + 0x7d4, 0);
}
