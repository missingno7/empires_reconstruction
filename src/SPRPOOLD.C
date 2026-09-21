/* F_78C3 -- draw up to four sprites from the pool at DS:C0EE, one per set
   bit of the mask.  The bytes of this draft were already established by the
   pilots; what is new is that the two symbols are spelled by the DECLARED
   naming convention (`gc0ee`, `gfx_blit_bitmap`) instead of the hand-chosen `pool`
   and `draw`, which no binding, frame or convention could decide. */
struct H { char pad[0x28]; unsigned ofs[4]; };
extern struct H *gc0ee;                /* DS:C0EE, far under -mc */
extern void gfx_blit_bitmap(char near *dst, int b, char far *src);

void sprite_pool_draw_masked(char near *dst, int b, unsigned mask)
{
    int k;
    for (k = 0; k < 4; k++)
        if (mask & (1 << k))
            gfx_blit_bitmap(dst + k * 14, b, (char far *)gc0ee + gc0ee->ofs[k] + 2);
}
