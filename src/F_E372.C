/* F_E372 -- fold five fields of a 14-byte record into one flag word and hand
   it, with the record's own glyph, to fc898.  Entry 1E472, 174 bytes.

   E377  8B7E04              the parameter is copied into DI, so it is a
                              register variable; SI takes the running sum,
                              which means TC 2.0 gave SI to the BODY's
                              register local and DI to the register PARAMETER.
   E37A  BA0E00 F7E2 8BD8 81C31BC9 1E 07 26807F0900
                             gc91b[i].f9 -- a struct member through an array
                              with a runtime index, so the far pointer is
                              materialised OFFSET first (add bx,offset;
                              push ds; pop es), the shape F_656C's second
                              attempt established.
   E38E  7405 BE8000 EB02 33F6
                             `v = cond ? 0x80 : 0` writes the destination
                              REGISTER directly in both arms.  The four that
                              follow are `v += cond ? K : 0` and go through AX
                              (mov ax,K / xor ax,ax) and then `add si,ax`.
   E40E  8A85E42F            g2fe4[i] is a plain char array indexed by the
                              register: TC folds base+index into ONE near DS
                              displacement and materialises no segment at all.
                              That is the same folding that made F_49F0's
                              first attempt 12 bytes short -- it happens for a
                              scalar array element and not for a struct
                              member. */

struct S {                              /* 14 bytes, `mov dx,0xE; mul dx` */
    char f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, fa, fb, fc, fd;
};

extern int fc898();

extern struct S gc91b[];                /* DS:C91B */
extern char g2fe4[];                    /* DS:2FE4 */

fe372(i)
register int i;
{
    register int v;

    v = gc91b[i].f9 ? 0x80 : 0;
    v += gc91b[i].fa ? 0x40 : 0;
    v += gc91b[i].f5 ? 0x20 : 0;
    v += gc91b[i].fb ? 0x10 : 0;
    v += gc91b[i].f1 & 0xf;
    fc898(g2fe4[i] + 0x20, v);
}
