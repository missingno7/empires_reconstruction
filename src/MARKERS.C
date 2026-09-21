#include "VIDEO.H"
/* src/MARKERS.C: Marker cell drawing.
   One translation unit; the sections below were the separate member
   sources of grouped module C_984C_9871 and keep their original ids. */

/* ---- F_984C (original code at 0x984C) ---- */
extern void gfx_color_select(), gfx_clear_rect();
void score_panel_clear(void) { gfx_color_select(15); gfx_clear_rect(0, 0x168, 150, 50); gfx_color_select(0); }


/* ---- F_9871 (original code at 0x9871) ---- */
extern int score_panel_x, score_panel_y;
void roundend_draw_marker(void) { gfx_wipe_rect(0, 0x168, 40, 29, score_panel_x, score_panel_y); }
