/* F_025B -- normalise a far pointer: carry the top 12 bits of the offset into
   the segment and keep the low nibble.  Both parameters are promoted into
   register variables (rule 12).  Both assignments are PLAIN, not compound:
   `s = (o >> 4) + s` computes into AX and moves it to DI, where `s += o >> 4`
   would have added straight into DI (rule 9's spelling rule, in the direction
   opposite to the one F_E372 witnesses). */
#define MK_FP(seg, ofs) ((void far *) (((unsigned long) (seg) << 16) | (unsigned) (ofs)))

char far *f025b(o, s)
register unsigned o;
register unsigned s;
{
    s = (o >> 4) + s;
    o = o & 15;
    return ((char far *) MK_FP(s, o));
}
