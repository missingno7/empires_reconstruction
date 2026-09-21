/* src/SCORE.C: Score panel.
   One translation unit; the sections below were the separate member
   sources of grouped module C_9962_99E2 and keep their original ids. */

#include "VIDEO.H"
extern void gfx_wipe_rect();
extern int score_panel_x, score_panel_y;

/* ---- F_9962 (original code at 0x9962) ---- */
void f9962(void) { gfx_copy_rect_split_flip_v(4,360,32,28,80,360); gfx_wipe_rect(80,362,30,29,score_panel_x+6,score_panel_y); }


/* ---- F_99A2 (original code at 0x99A2) ---- */
void f99a2(void) { gfx_copy_rect_split(4,360,32,28,80,360); gfx_wipe_rect(80,362,30,29,score_panel_x+5,score_panel_y); }


/* ---- F_99E2 (original code at 0x99E2) ---- */
struct score_pos { int a,b; };
extern struct score_pos g1271[];
void score_set_position(int i) { score_panel_x=g1271[i].a; score_panel_y=g1271[i].b; }
