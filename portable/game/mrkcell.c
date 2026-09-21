/* mrkcell.c -- portable port of src/MRKCELL.C. */
#include "game.h"

void marker_cell_draw_highlight(void)
{
    gfx_copy_rect_flip_h(0, 360, 40, 29, 80, 360);
    gfx_wipe_rect(80, 360, 40, 29, score_panel_x, score_panel_y);
}

void marker_cell_draw_plain(void)
{
    gfx_copy_rect_flip_v(1, 360, 40, 29, 80, 360);
    gfx_wipe_rect(80, 360, 40, 29, score_panel_x, score_panel_y);
}
