/* F_48BE -- patch the blitter at IP 0x039C in place with the variant record
   for the current display mode.  The destination is a far pointer to CODE,
   widened from the near function with a cast, which is why it pushes
   `cs` and not a segment fixup. */
extern int resource_load_record();
extern void far *memmove();
extern void runtime_base();
extern char display_mode;                      /* DS:BFCD, the display mode */
extern char far *ui_gfx_shadow_a;                 /* DS:C5C6 offset, DS:C5C8 segment */

void blitter_patch_variant()
{
    register int n;

    if (display_mode == 5) {
        n = resource_load_record(2);
        memmove((char far *) runtime_base, ui_gfx_shadow_a, n);
    } else if (display_mode == 2) {
        n = resource_load_record(3);
        memmove((char far *) runtime_base, ui_gfx_shadow_a, n);
    }
}
