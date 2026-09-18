/* F_E1F2 -- scale one record's bar into the 0..0x3F range, pack the record's
   first byte above it, and hand the pair to fc898.  Entry 1E2F2, 112 bytes.

   E22B  33D2 F7F3           `xor dx,dx; div bx` is the UNSIGNED divide, so
              the running value is an unsigned int; a signed int would have
              been `cwd; idiv` and a long a runtime helper.  The same reading
              covers `mul si` at E21B.
   E21F  8BC6 057F00 03F0    `v += v + 0x7f`: the right side is computed whole
              into AX and then added back, which is why v appears twice.
   E232  BA3F00 2BD0 8BF2    `0x3f - v / 0xfe` has to borrow DX because the
              quotient already occupies AX; the same subtraction at E211,
              where AX is free, goes straight into SI.
   E216 / E250  gca50[i] and g2fe4[i] are plain char arrays indexed by the
              register parameter, so both fold to a single near DS
              displacement (0xCA50 written as di-0x35b0), while gc91b[i].f8
              at E203 keeps the far form -- the scalar-element vs
              struct-member boundary F_E372 and F_D8F0 also carry. */

struct S {                              /* 14 bytes, `mov dx,0xE; mul dx` */
    char f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, fa, fb, fc, fd;
};

extern int fc898();

extern struct S gc91b[];                /* DS:C91B */
extern char gca50[];                    /* DS:CA50 */
extern char g2fe4[];                    /* DS:2FE4 */

fe1f2(i)
register int i;
{
    register unsigned v;

    v = 0x3f - (gc91b[i].f8 & 0x3f);
    v = gca50[i] * v;
    v += v + 0x7f;
    v = 0x3f - v / 0xfe;
    v |= gc91b[i].f0 << 6;
    fc898(g2fe4[i] + 0x40, v);
}
