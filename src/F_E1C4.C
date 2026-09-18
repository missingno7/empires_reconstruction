/* F_E1C4 -- re-arm one voice: reset the two shared registers, then push every
   one of the voice's six parameter groups to the OPL in a fixed order. */
extern void fe420();
extern void fe262();
extern void fe1f2();
extern void fe27b();
extern void fe2d6();
extern void fe324();
extern void fe372();
extern void fe44b();

void fe1c4(v)
register int v;
{
    fe420();
    fe262();
    fe1f2(v);
    fe27b(v);
    fe2d6(v);
    fe324(v);
    fe372(v);
    fe44b(v);
}
