/* F_0281 -- allocate the 488-row screen buffer for the current mode, normalise
   its base to a paragraph and fill the DS:3924 row table with one normalised
   far pointer per row, then load the mode's palette.  The row width is a
   register variable (SI) declared before the loop counter (DI), rule 12; the
   size is `(long)w * 488 + 16` and the `cwd` before the long multiply makes
   `w` a SIGNED int widened to long (rule 16's shape). */
#define MK_FP(seg, ofs) ((void far *) (((unsigned long) (seg) << 16) | (unsigned) (ofs)))
#define FP_SEG(fp) ((unsigned) ((unsigned long) (void far *) (fp) >> 16))

extern char far *farmalloc();
extern char far *f025b();
extern void f039c();
extern void f0232();
extern void f01bc();
extern char gbfcd;                      /* DS:BFCD, the display mode */
extern char far *g40ca;                 /* DS:40CA offset, DS:40CC segment */
extern char far *g3924[];               /* DS:3924 */
extern unsigned char g11e[];            /* DS:011E */
extern unsigned char g41e[];            /* DS:041E */

void f0281()
{
    char far *q;
    unsigned s;
    register int w;
    register int i;

    if (gbfcd == 5)
        w = 0x140;
    else if (gbfcd == 2)
        w = 0x50;
    else
        w = 0xa0;
    g40ca = farmalloc((long) w * 488 + 16);
    s = FP_SEG(g40ca) + 1;
    g40ca = MK_FP(s, 0);
    q = f025b(g40ca);
    for (i = 0; i < 488; i++) {
        g3924[i] = q;
        q = f025b(q + w);
    }
    f039c();
    f0232();
    if (gbfcd == 5)
        f01bc(g11e);
    else if (gbfcd == 4)
        f01bc(g41e);
}
