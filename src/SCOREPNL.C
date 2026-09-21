#include "VIDEO.H"
/* score_panel_x is an int everywhere else (ANIMFRAM.C, MARKERS.C, MRKCELL.C,
   ROUNDEND.C, SCORE.C); here it is deliberately redeclared `char *` (far
   under -mc) so the single argument reads the 4 bytes at &score_panel_x --
   the adjacent int score_panel_y falls in the high word -- packing the last
   two gfx_wipe_rect() parameters (x2,y2) into one far-pointer push. This is
   a genuine, load-bearing type pun, not a stray declaration; unifying it to
   `int` would drop the y2 argument. */
extern void gfx_wipe_rect(); extern char *score_panel_x; score_panel_draw(){gfx_copy_rect_flip_h(0,0x168,40,29,80,0x168);gfx_copy_rect_flip_v(80,0x168,40,29,160,0x168);gfx_wipe_rect(160,0x168,40,29,score_panel_x);}
