extern void f03ba();extern void f03b4();
extern int score_panel_x,score_panel_y;
void marker_cell_draw_highlight(void)
{
 f03ba(0,360,40,29,80,360);
 f03b4(80,360,40,29,score_panel_x,score_panel_y);
}

extern int f03b7();extern void f03b4();
extern int score_panel_x,score_panel_y;
void marker_cell_draw_plain(void)
{
 f03b7(1,360,40,29,80,360);
 f03b4(80,360,40,29,score_panel_x,score_panel_y);
}
