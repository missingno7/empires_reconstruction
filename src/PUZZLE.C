/* src/PUZZLE.C: Grid puzzle: dealing, placing and checking pieces.
   One translation unit; the sections below were the separate member
   sources of grouped module C_8A37_969D and keep their original ids. */

#include "GC316.H"
#include "VIDEO.H"

/* puzzle_held_piece is declared here as plain char (see GC316.H's header
   comment); F_8BAB reinterprets it as the tile struct through this macro
   instead of a second, conflicting extern declaration. */
#define GC132 (*(struct gc316_tile *)&puzzle_held_piece)

extern int puzzle_piece_total;
extern char puzzle_held_piece, gc133, puzzle_solved_flag, puzzle_cursor_col, puzzle_cursor_row;
extern int rand(void);
extern int slot_is_new_game(void);
extern void hud_draw_meter(void);
extern int music_track_handle;
extern int cur_color_index_get(),sprite_sheet_index_get(),menu_list_active(),keyboard_chain_active(),campaign_node_index();
extern void keyboard_chain_enable(void);
extern void keyboard_buffer_drain(void);
extern int puzzle_display_init(),keyboard_read_blocking_hotkeys(),f9440();
extern void menu_list_disable(void);
extern void tutorial_hint_dialog_show(int);
extern void menu_list_enable(void);
extern void timer_wait_ticks(int n);
extern int puzzle_cell_backing_swap(),puzzle_cell_highlight_draw(),puzzle_clear_cell(),puzzle_draw_piece();
extern void stream_control_block_arm();
extern void timer_deadline_arm();
extern void player_select_restart_confirm();
extern void f9402(int i);
extern int timer_deadline_reached(),keyboard_poll_nonblocking(),puzzle_check_solved(),puzzle_draw_tray_piece();
extern void puzzle_free_resources(void);
extern void sprite_sheet_select();
extern void resource_record_cache_reset();
extern void sound_stop_reset(void);
extern void gfx_color_select(int n);
extern void keyboard_chain_disable(void);
extern int tick_div8(), resource_load_record();
extern char far * strcpy();
extern void gfx_copy_rect();extern void gfx_blit_bitmap();extern void gfx_wipe_rect();extern void gfx_box();
extern void resource_load_record_alloc();
extern char far *gdc4,far *gdc8,far *ui_gfx_shadow_a;
extern char gc136[][240];
extern void free();
extern unsigned far *g0dc8;
extern void hud_prompt_message_draw();
extern int hud_prompt_confirm_draw();
extern char display_mode;
extern void gfx_bar();extern void rect_border_draw();

/* ---- F_8A37 (original code at 0x8A37) ---- */
/* F_8A37 -- clear the 4 x 6 slot grid.  Two statements, so two full address
   computations; i is si and j is di (SI is allocated before DI). */
void puzzle_clear_grid(void)
{
    register int i, j;

    puzzle_piece_total = 0;
    puzzle_solved_flag = 0;
    for (i = 0; i < 4; i++)
        for (j = 0; j < 6; j++) {
            puzzle_grid[i][j].kind = 0xff;
            puzzle_grid[i][j].rot = 0;
        }
    puzzle_held_piece = 0xff;
    gc133 = 0;
    puzzle_cursor_row = 0;
    puzzle_cursor_col = 0;
}


/* ---- F_8AA2 (original code at 0x8AA2) ---- */
/* F_8AA2 -- deal two entries into the 4x6 slot grid gC316: for each of two
   rounds take the next free index gC130, pick a random offset into the
   remaining free cells, walk the grid cyclically that many free cells on,
   stamp the index there and give it a random 0..3 attribute when fACE7()
   says so.  Returns whether the grid is now full. */
int puzzle_deal_pieces(void)
{
    register int row;
    register int col;
    int slot, r, k, i;   /* TC 2.0 lays locals out in REVERSE declaration
                            order: the LAST declared gets [bp-2] */

    row = 0;
    col = -1;
    for (i = 0; i < 2; i++) {
        if ((slot = puzzle_piece_total++) >= 0xc)
            return -1;
        r = rand() % (0xc - slot);
        for (k = 0; k <= r; k++)
            do {
                if (++col == 3) {
                    col = 0;
                    if (++row == 4)
                        row = 0;
                }
            } while ((unsigned char) puzzle_grid[row][col].kind != 0xff);
        puzzle_grid[row][col].kind = slot;
        if (slot_is_new_game())
            puzzle_grid[row][col].rot = rand() % 4;
        else
            puzzle_grid[row][col].rot = 0;
    }
    hud_draw_meter();
    return puzzle_piece_total == 0xc;
}


/* ---- F_8BA5 (original code at 0x8BA5) ---- */
puzzle_piece_count(){return puzzle_piece_total;}


/* ---- F_8BAB (original code at 0x8BAB) ---- */
/* Grid selection, piece placement, and completion state machine. */
int puzzle_run(void)
{
 int key,quit,redraw,flash,saved_input,saved_timer,saved_mode,saved_cursor,level,stage,saved_state;
 register int row,col;
 quit=0;redraw=1;flash=1;stage=0;saved_state=music_track_handle;
 saved_input=cur_color_index_get();saved_timer=sprite_sheet_index_get();saved_cursor=menu_list_active();saved_mode=keyboard_chain_active();
 keyboard_chain_enable();keyboard_buffer_drain();level=campaign_node_index();puzzle_display_init();
 if(slot_is_new_game()) tutorial_hint_dialog_show(8);else tutorial_hint_dialog_show(6);
 while(!quit) {
  if(puzzle_solved_flag) {
   menu_list_disable();
   while(stage<2) {
    timer_wait_ticks(118);keyboard_buffer_drain();
    if((key=keyboard_read_blocking_hotkeys())==13) {if(++stage<2) f9440(stage);}
    else if(key==27) stage=2;
   }
   if(saved_cursor) menu_list_enable();
   quit=1;
  }
  while(GC132.kind==-1&&!quit) {
   puzzle_cell_backing_swap(puzzle_cursor_row,puzzle_cursor_col,0,0);puzzle_cell_highlight_draw(puzzle_cursor_row,puzzle_cursor_col,1,0);puzzle_clear_cell(puzzle_cursor_row,puzzle_cursor_col);
   row=puzzle_cursor_row;col=puzzle_cursor_col;key=keyboard_read_blocking_hotkeys();
   switch(key) {
   case 0x148:if(--puzzle_cursor_row<0) puzzle_cursor_row=3;break;
   case 0x150:if(++puzzle_cursor_row>=4) puzzle_cursor_row=0;break;
   case 0x14b:if(--puzzle_cursor_col<0) puzzle_cursor_col=5;break;
   case 0x14d:if(++puzzle_cursor_col>=6) puzzle_cursor_col=0;break;
   case 13:
    if(puzzle_grid[puzzle_cursor_row][puzzle_cursor_col].kind==-1) stream_control_block_arm(23);
    else {
     GC132=puzzle_grid[puzzle_cursor_row][puzzle_cursor_col];puzzle_grid[puzzle_cursor_row][puzzle_cursor_col].kind=-1;
     redraw=1;flash=1;timer_deadline_arm(118);stream_control_block_arm(14);f9402(1);
    }
    break;
   case 27:player_select_restart_confirm();break;
   }
   puzzle_cell_backing_swap(row,col,0,1);
   if(GC132.kind==-1) puzzle_clear_cell(row,col);
   else puzzle_draw_piece(puzzle_grid[puzzle_cursor_row][puzzle_cursor_col],puzzle_cursor_row,puzzle_cursor_col);
  }
  while(GC132.kind!=-1&&!quit) {
   if(redraw) {
    puzzle_cell_backing_swap(puzzle_cursor_row,puzzle_cursor_col,0,0);puzzle_draw_piece(GC132,puzzle_cursor_row,puzzle_cursor_col);
    puzzle_cell_backing_swap(puzzle_cursor_row,puzzle_cursor_col,1,0);puzzle_cell_highlight_draw(puzzle_cursor_row,puzzle_cursor_col,flash,1);puzzle_clear_cell(puzzle_cursor_row,puzzle_cursor_col);redraw=0;
   }
   if(timer_deadline_reached()) {
    puzzle_cell_backing_swap(puzzle_cursor_row,puzzle_cursor_col,1,1);flash=!flash;
    puzzle_cell_highlight_draw(puzzle_cursor_row,puzzle_cursor_col,flash,1);puzzle_clear_cell(puzzle_cursor_row,puzzle_cursor_col);timer_deadline_arm(118);
   }
   if(keyboard_poll_nonblocking()) {
    row=puzzle_cursor_row;col=puzzle_cursor_col;key=keyboard_read_blocking_hotkeys();
    switch(key) {
    case 0x148:if(--puzzle_cursor_row<0) puzzle_cursor_row=3;break;
    case 0x150:if(++puzzle_cursor_row>=4) puzzle_cursor_row=0;break;
    case 0x14b:if(--puzzle_cursor_col<0) puzzle_cursor_col=5;break;
    case 0x14d:if(++puzzle_cursor_col>=6) puzzle_cursor_col=0;break;
    case 'F':case 'f':if(slot_is_new_game()) {if(++GC132.rot>3) GC132.rot=0;}break;
    case 13:
     if(puzzle_grid[puzzle_cursor_row][puzzle_cursor_col].kind!=-1) stream_control_block_arm(23);
     else {puzzle_grid[puzzle_cursor_row][puzzle_cursor_col]=GC132;GC132.kind=-1;stream_control_block_arm(26);f9402(0);}
     break;
    case 27:player_select_restart_confirm();break;
    }
    if(GC132.kind!=-1) {puzzle_cell_backing_swap(row,col,0,1);puzzle_clear_cell(row,col);}
    else {
     puzzle_cell_backing_swap(row,col,1,1);puzzle_clear_cell(row,col);
     if(puzzle_solved_flag=puzzle_check_solved()) {keyboard_buffer_drain();f9440(stage);puzzle_draw_tray_piece(level);sound_stop_reset();stream_control_block_arm(27);}
    }
    redraw=1;
   }
  }
 }
 tutorial_hint_dialog_show(7);puzzle_free_resources();gfx_color_select(saved_input);sprite_sheet_select(saved_timer);
 if(!saved_mode) keyboard_chain_disable();
 keyboard_buffer_drain();resource_record_cache_reset(saved_state);return puzzle_solved_flag;
}


/* ---- F_90A6 (original code at 0x90A6) ---- */
puzzle_display_init(){int n,k;register int i,j;n=campaign_node_index();resource_load_record_alloc(tick_div8()*2+slot_is_new_game()+4201,&gdc4);resource_load_record(4159);gfx_blit_bitmap(8,16,ui_gfx_shadow_a);gfx_wipe_rect(8,16,304,144,8,202);resource_load_record(tick_div8()*8+campaign_node_index()+slot_is_new_game()*4+4161);gfx_blit_bitmap(0,344,ui_gfx_shadow_a+((int *)ui_gfx_shadow_a)[0]+2);gfx_copy_rect(140,44,ui_gfx_shadow_a+((int *)ui_gfx_shadow_a)[1]+2,0);for(k=0;k<2;k++)strcpy(gc136[k],ui_gfx_shadow_a+((int *)ui_gfx_shadow_a+2)[k]+2);resource_load_record_alloc(4160,&gdc8);if(puzzle_solved_flag)f9440(1);else f9402(puzzle_held_piece!=-1);for(i=0;i<4;i++)for(j=0;j<6;j++)puzzle_draw_piece(puzzle_grid[i][j],i,j);for(i=0;i<n;i++)puzzle_draw_tray_piece(i);if(puzzle_solved_flag)puzzle_draw_tray_piece(n);gfx_box(8,16,304,144);}


/* ---- F_9259 (original code at 0x9259) ---- */
/* F_9259 -- release the two far blocks at DS:0DC8 and DS:0DC4 if they are
   held.  TC 2.0 tests a far pointer against NULL by OR-ing its two halves
   (rule 16), and clearing one writes the SEGMENT half first.  The callee is
   spelled BY THE CONVENTION: the image calls IP 0F3F7h, which is the FFREE
   module's base and NOT `_farfree` -- the declared binding puts farfree at
   0F6C3h, +0x2CC into the same module. */
void puzzle_free_resources()
{
    if (gdc8) {
        free(gdc8);
        gdc8 = 0;
    }
    if (gdc4) {
        free(gdc4);
        gdc4 = 0;
    }
}


/* ---- F_929E (original code at 0x929E) ---- */
struct piece_desc{char a,b;};puzzle_draw_piece(r,a,b) struct piece_desc r;int a,b;{int u,v;register int x,y;if(b<3){x=b*36+28;y=a*28+43;}else{b-=3;x=b*32+140;y=a*24+54;}if(r.a==-1)gfx_wipe_rect(x,y+186,32,24,x,y);else{u=(r.a%3)*32;v=(r.a/3)*24+344;switch(r.b){case 0:gfx_wipe_rect(u,v,32,24,x,y);break;case 1:gfx_copy_rect_flip_h(u,v,32,24,x,y);break;case 2:gfx_copy_rect_flip_hv(u,v,32,24,x,y);break;case 3:gfx_copy_rect_flip_v(u,v,32,24,x,y);break;}}}


/* ---- F_93AA (original code at 0x93AA) ---- */
puzzle_draw_tray_piece(i) register int i; {gfx_copy_rect(0x108,27+(i<<5),gdc4+((unsigned *)gdc4)[i]+2,0);gfx_box(0x108,27+(i<<5),28,24);}


/* ---- F_9402 (original code at 0x9402) ---- */
void f9402(int i) { register int flag; flag=i && !slot_is_new_game(); hud_prompt_message_draw((char far *)g0dc8 + g0dc8[i] + 2,flag); }


/* ---- F_9440 (original code at 0x9440) ---- */
f9440(i) int i; {hud_prompt_confirm_draw(gc136[i],0,11,1,1);}


/* ---- F_9466 (original code at 0x9466) ---- */
puzzle_cell_backing_swap(a,b,c,d) int a,b,c,d;{register int x,y;int u,v;if(b<3){x=b*36+26;y=a*28+41;}else{b-=3;x=(b<<5)+138;y=a*24+52;}switch(c){case 0:u=96;v=0x158;break;case 1:u=140;v=0x158;break;}if(!d)gfx_wipe_rect(x,y,42,30,u,v);else gfx_wipe_rect(u,v,42,30,x,y);}


/* ---- F_950C (original code at 0x950C) ---- */
puzzle_cell_highlight_draw(a,b,c,d) int a,b,c,d;{int y,w,h,old,n1,n2,n3;register int i,x;if(b<3){x=b*36+28;y=a*28+43;}else{b-=3;x=b*32+140;y=a*24+54;}w=32;h=24;old=cur_color_index_get();n1=c?2:0;n2=d?5:0;n3=d?2:0;if(display_mode==2)gfx_color_select(3);else if(display_mode==3)gfx_color_select(0);else gfx_color_select(4);for(i=0;i<n1;i++){w+=2;h+=2;rect_border_draw(--x,--y,w,h);}gfx_color_select(8);for(i=0;i<n2;i++)gfx_vline(x+w+i,y+2,h);for(i=0;i<n3;i++)gfx_bar(x+5,y+h+i,w);gfx_color_select(old);}


/* ---- F_963E (original code at 0x963E) ---- */
puzzle_clear_cell(a,b) int a; register int b; {register int x; int y;if(b<3){x=b*36+26;y=a*28+41;}else{b-=3;x=(b<<5)+138;y=a*24+52;}gfx_box(x,y,42,30);}


/* ---- F_969D (original code at 0x969D) ---- */
puzzle_check_solved(){int r,n;register int i,j;r=1;switch(puzzle_grid[0][3].rot){case 0:for(i=0,n=0;i<4;i++){for(j=3;j<6;j++,n++){if(puzzle_grid[i][j].kind!=n||puzzle_grid[i][j].rot!=0)r=0;}}break;case 1:for(i=0,n=0;i<4;i++){for(j=5;j>=3;j--,n++){if(puzzle_grid[i][j].kind!=n||puzzle_grid[i][j].rot!=1)r=0;}}break;case 2:for(i=3,n=0;i>=0;i--){for(j=5;j>=3;j--,n++){if(puzzle_grid[i][j].kind!=n||puzzle_grid[i][j].rot!=2)r=0;}}break;case 3:for(i=3,n=0;i>=0;i--){for(j=3;j<6;j++,n++){if(puzzle_grid[i][j].kind!=n||puzzle_grid[i][j].rot!=3)r=0;}}break;}return r;}
