/* F_CAD0 -- speaker gate on.  asm body; no parameter, so the frame is only
   there if the module was compiled -k.  @FLAGS records that as a per-module
   flag, the way Cosmore's makefile carries per-module -d-/-1-. */
/*@FLAGS -k*/
void fcad0()
{
    asm in  al,61h
    asm or  al,3
    asm out 61h,al
}
