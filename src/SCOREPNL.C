#include "VIDEO.H"
extern void gfx_wipe_rect(); extern char *score_panel_x; score_panel_draw(){gfx_copy_rect_flip_h(0,0x168,40,29,80,0x168);gfx_copy_rect_flip_v(80,0x168,40,29,160,0x168);gfx_wipe_rect(160,0x168,40,29,score_panel_x);}
