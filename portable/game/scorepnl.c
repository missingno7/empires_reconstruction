/* scorepnl.c -- portable port of src/SCOREPNL.C.
 *
 * The historical source redeclares score_panel_x as `char *` (far under
 * -mc) purely so the single gfx_wipe_rect() argument reads the 4 bytes at
 * &score_panel_x (score_panel_y falls in the high word), packing the last
 * two parameters (x2,y2) into one far-pointer push -- a K&R
 * no-prototype-checking calling-convention trick, not a semantic choice.
 * gfx.h already declares gfx_wipe_rect with explicit dx,dy dos_int
 * parameters, so the port just passes score_panel_x/score_panel_y
 * directly, matching MARKERS.C's/MRKCELL.C's ordinary 6-argument calls.
 * PORT: paragraph/far-pointer-argument-packing trick retired
 * (tu-porting-rules.md sec 6); semantics unchanged.
 */
#include "game.h"

dos_int score_panel_draw(void)
{
    gfx_copy_rect_flip_h(0, 0x168, 40, 29, 80, 0x168);
    gfx_copy_rect_flip_v(80, 0x168, 40, 29, 160, 0x168);
    gfx_wipe_rect(160, 0x168, 40, 29, score_panel_x, score_panel_y);
    return 0;   /* PORT: value unused (K&R implicit int) */
}
