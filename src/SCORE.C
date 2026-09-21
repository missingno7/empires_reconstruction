/* src/SCORE.C: Score panel.
   One translation unit; the sections below were the separate member
   sources of grouped module C_9962_99E2 and keep their original ids. */

extern void f03c3(), f03c0(), f03b4();
extern int score_panel_x, score_panel_y;

/* ---- F_9962 (original code at 0x9962) ---- */
void f9962(void) { f03c3(4,360,32,28,80,360); f03b4(80,362,30,29,score_panel_x+6,score_panel_y); }


/* ---- F_99A2 (original code at 0x99A2) ---- */
void f99a2(void) { f03c0(4,360,32,28,80,360); f03b4(80,362,30,29,score_panel_x+5,score_panel_y); }


/* ---- F_99E2 (original code at 0x99E2) ---- */
struct R { int a,b; };
extern struct R g1271[];
void score_set_position(int i) { score_panel_x=g1271[i].a; score_panel_y=g1271[i].b; }
