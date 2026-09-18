/* F_78C3 -- draw up to four sprites from the pool at DS:C0EE, one per set
   bit of the mask.  The bytes of this draft were already established by the
   pilots; what is new is that the two symbols are spelled by the DECLARED
   naming convention (`gc0ee`, `f03c9`) instead of the hand-chosen `pool`
   and `draw`, which no binding, frame or convention could decide. */
struct H { char pad[0x28]; unsigned ofs[4]; };
extern struct H *gc0ee;                /* DS:C0EE, far under -mc */
extern void f03c9(char near *dst, int b, char far *src);

void f78c3(char near *dst, int b, unsigned mask)
{
    int k;
    for (k = 0; k < 4; k++)
        if (mask & (1 << k))
            f03c9(dst + k * 14, b, (char far *)gc0ee + gc0ee->ofs[k] + 2);
}
