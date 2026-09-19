/* F_D8F0 -- rebuild the nine-slot panel from the record at gc5da+8.  Entry
   1D9F0, 171 bytes.  One far-pointer local (bp-04, segment at bp-02), one
   register variable (SI = i), no parameters.

   D8F7  C41EDAC5 83C308 8C46FE 895EFC
                             p = gc5da + 8: `les` loads the far pointer whole,
                              the constant lands on the OFFSET, and the store
                              writes the SEGMENT word first.
   D91F  8B16DCC5 A1DAC5 051100 52 50
                             the same pointer + 0x11 as an ARGUMENT is built
                              the other way: two loads, add on the offset,
                              push segment then offset.
   D95A  BB3800 52 F7E3 8BD8 81C34430 1E 07 58 268907
                             `g3044[*p].a = <rhs>` -- the RHS is computed
                              first, PUSHED across the address computation and
                              popped back, which is TC 2.0's order for a store
                              whose left side needs a multiply.
   D962  81C34430 1E 07 268907     vs
   D978  BB1A30 8CD9 03D8 51 53
                             the two far-pointer shapes side by side in ONE
                              function: indexing an array of 0x38-byte
                              structs builds the OFFSET first and loads the
                              segment last, while a far CAST of an array base
                              plus an index materialises the pointer first and
                              adds the index after.  F_656C's negative test
                              named this distinction; here the original itself
                              carries both, 22 bytes apart.
   D988  C684ABC601          gc6ab[i] = 1 -- a plain char array indexed by the
                              register folds to one near DS displacement
                              (0xC6AB written as si-0x3955). */

struct U {                              /* 56 bytes, `mov dx,0x38; mul dx` */
    int a;
    char rest[0x36];
};

extern int memset(), movmem(), fda66();

extern char far *gc5da;                 /* DS:C5DA, segment at DS:C5DC */
extern char gc6ab[];                    /* DS:C6AB */
extern char gca62[];                    /* DS:CA62 */
extern char g301a[];                    /* DS:301A, stride 0x38 */
extern struct U g3044[];                /* DS:3044, stride 0x38 */

fd8f0()
{
    char far *p;                        /* bp-04 */
    register int i;                     /* si */

    p = gc5da + 8;
    memset(gc6ab, 0, 9);
    movmem(gc5da + 0x11, gca62, 9);
    for (i = 0; i < 9; i++, p++) {
        if (*p != -1) {
            g3044[*p].a = 0x3f - gc5da[i + 0x1a] * 9;
            fda66(i, (char far *)g301a + *p * 0x38);
            gc6ab[i] = 1;
        }
    }
}
