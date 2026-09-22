/* src/PUZZLE.C: Grid puzzle: dealing, placing and checking pieces. */
#include "game.h"

/* PORT: GC132 (src/PUZZLE.C: `#define GC132 (*(struct gc316_tile *)&puzzle_held_piece)`)
   reinterprets puzzle_held_piece TOGETHER WITH the immediately following
   byte gc133 as one 2-byte struct gc316_tile.  Generator fix landed
   (supervisor decision): portable/generated/game_state.h now has a real
   `extern struct gc316_tile gc132_tile;` plus
   `#define puzzle_held_piece (gc132_tile.kind)` and
   `#define gc133 (gc132_tile.rot)`, so `&puzzle_held_piece` itself now
   expands to `&(gc132_tile.kind)` -- the macro below keeps working
   unmodified and is safe (kind is gc316_tile's first field, so the cast
   just recovers gc132_tile itself). */
#define GC132 (*(struct gc316_tile *)&puzzle_held_piece)

/* ---- F_8A37 (original code at 0x8A37) ---- */
/* F_8A37 -- clear the 4 x 6 slot grid.  Two statements, so two full address
   computations; i is si and j is di (SI is allocated before DI). */
void puzzle_clear_grid(void)
{
    dos_int i, j;

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
dos_int puzzle_deal_pieces(void)
{
    dos_int row;
    dos_int col;
    dos_int slot, r, k, i;   /* TC 2.0 lays locals out in REVERSE declaration
                            order: the LAST declared gets [bp-2] */

    row = 0;
    col = -1;
    for (i = 0; i < 2; i++) {
        if ((slot = puzzle_piece_total++) >= 0xc)
            return -1;
        /* PORT: rand() -- Turbo C's LCG differs from any modern libc's; see
           docs/portable/int-semantics-inventory.md sec "RNG-touching
           clusters".  Not resolved here (out of this agent's scope);
           reported as-is. */
        r = rand() % (0xc - slot);
        for (k = 0; k <= r; k++)
            do {
                if (++col == 3) {
                    col = 0;
                    if (++row == 4)
                        row = 0;
                }
            } while ((dos_uchar) puzzle_grid[row][col].kind != 0xff);
        puzzle_grid[row][col].kind = (dos_char)slot;
        if (slot_is_new_game())
            puzzle_grid[row][col].rot = (dos_char)(rand() % 4);
        else
            puzzle_grid[row][col].rot = 0;
    }
    hud_draw_meter();
    return puzzle_piece_total == 0xc;
}


/* ---- F_8BA5 (original code at 0x8BA5) ---- */
dos_int puzzle_piece_count(void) { return puzzle_piece_total; }


/* ---- F_8BAB (original code at 0x8BAB) ---- */
/* Grid selection, piece placement, and completion state machine. */
dos_int puzzle_run(void)
{
 dos_int key,quit,redraw,flash,saved_input,saved_timer,saved_mode,saved_cursor,level,stage,saved_state;
 dos_int row,col;
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
   else puzzle_draw_piece(*(struct piece_desc *)&puzzle_grid[puzzle_cursor_row][puzzle_cursor_col],puzzle_cursor_row,puzzle_cursor_col);
  }
  while(GC132.kind!=-1&&!quit) {
   if(redraw) {
    puzzle_cell_backing_swap(puzzle_cursor_row,puzzle_cursor_col,0,0);puzzle_draw_piece(*(struct piece_desc *)&GC132,puzzle_cursor_row,puzzle_cursor_col);
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
     if((puzzle_solved_flag=(dos_char)puzzle_check_solved())) {keyboard_buffer_drain();f9440(stage);puzzle_draw_tray_piece(level);sound_stop_reset();stream_control_block_arm(27);}
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
dos_int puzzle_display_init(void)
{
    dos_int n, k;
    dos_int i, j;

    n = campaign_node_index();
    resource_load_record_alloc(tick_div8() * 2 + slot_is_new_game() + 4201, (uint8_t **)&gdc4);
    resource_load_record(4159);
    gfx_blit_bitmap(8, 16, ui_gfx_shadow_a);
    gfx_wipe_rect(8, 16, 304, 144, 8, 202);
    resource_load_record(tick_div8() * 8 + campaign_node_index() + slot_is_new_game() * 4 + 4161);
    gfx_blit_bitmap(0, 344, ui_gfx_shadow_a + ((dos_int *)ui_gfx_shadow_a)[0] + 2);
    gfx_copy_rect(140, 44, ui_gfx_shadow_a + ((dos_int *)ui_gfx_shadow_a)[1] + 2, 0);
    /* PORT: strcpy is the standard <string.h> one (tu-porting-rules.md
       sec 4); dos_char/uint8_t are distinct types from plain `char` even
       though all three are 8-bit, so the pointer casts below are required
       to call it -- not a semantic change.  MSVC's strcpy deprecation
       warning (C4996, "consider strcpy_s") is suppressed locally: the
       historical semantics are an unbounded NUL-terminated copy, matching
       plain strcpy, not the bounds-checked strcpy_s. */
#pragma warning(push)
#pragma warning(disable: 4996)
    for (k = 0; k < 2; k++)
        strcpy((char *)gc136[k], (const char *)(ui_gfx_shadow_a + ((dos_int *)ui_gfx_shadow_a + 2)[k] + 2));
#pragma warning(pop)
    resource_load_record_alloc(4160, (uint8_t **)&gdc8);
    if (puzzle_solved_flag) f9440(1); else f9402(puzzle_held_piece != -1);
    for (i = 0; i < 4; i++)
        for (j = 0; j < 6; j++)
            puzzle_draw_piece(*(struct piece_desc *)&puzzle_grid[i][j], i, j);
    for (i = 0; i < n; i++)
        puzzle_draw_tray_piece(i);
    if (puzzle_solved_flag)
        puzzle_draw_tray_piece(n);
    gfx_box(8, 16, 304, 144);
    return 0;
}


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
dos_int puzzle_draw_piece(struct piece_desc r, dos_int a, dos_int b)
{
    dos_int u, v;
    dos_int x, y;

    if (b < 3) { x = b * 36 + 28; y = a * 28 + 43; }
    else { b -= 3; x = b * 32 + 140; y = a * 24 + 54; }
    if (r.a == -1)
        gfx_wipe_rect(x, y + 186, 32, 24, x, y);
    else {
        u = (r.a % 3) * 32;
        v = (r.a / 3) * 24 + 344;
        switch (r.b) {
        case 0: gfx_wipe_rect(u, v, 32, 24, x, y); break;
        case 1: gfx_copy_rect_flip_h(u, v, 32, 24, x, y); break;
        case 2: gfx_copy_rect_flip_hv(u, v, 32, 24, x, y); break;
        case 3: gfx_copy_rect_flip_v(u, v, 32, 24, x, y); break;
        }
    }
    return 0;
}


/* ---- F_93AA (original code at 0x93AA) ---- */
dos_int puzzle_draw_tray_piece(dos_int i)
{
    gfx_copy_rect(0x108, 27 + (i << 5), (const uint8_t *)(gdc4 + ((dos_uint *)gdc4)[i] + 2), 0);
    gfx_box(0x108, 27 + (i << 5), 28, 24);
    return 0;
}


/* ---- F_9402 (original code at 0x9402) ---- */
void f9402(dos_int i)
{
    dos_int flag;
    flag = i && !slot_is_new_game();
    hud_prompt_message_draw((dos_char *)((dos_char *)g0dc8 + g0dc8[i] + 2), flag);
}


/* ---- F_9440 (original code at 0x9440) ---- */
dos_int f9440(dos_int i)
{
    hud_prompt_confirm_draw(gc136[i], 0, 11, 1, 1);
    return 0;
}


/* ---- F_9466 (original code at 0x9466) ---- */
dos_int puzzle_cell_backing_swap(dos_int a, dos_int b, dos_int c, dos_int d)
{
    dos_int x, y;
    dos_int u, v;
    if (b < 3) { x = b * 36 + 26; y = a * 28 + 41; }
    else { b -= 3; x = (b << 5) + 138; y = a * 24 + 52; }
    switch (c) {
    case 0: u = 96; v = 0x158; break;
    case 1: u = 140; v = 0x158; break;
    default: u = 0; v = 0; break;
    }
    if (!d) gfx_wipe_rect(x, y, 42, 30, u, v);
    else gfx_wipe_rect(u, v, 42, 30, x, y);
    return 0;
}


/* ---- F_950C (original code at 0x950C) ---- */
dos_int puzzle_cell_highlight_draw(dos_int a, dos_int b, dos_int c, dos_int d)
{
    dos_int y, w, h, old, n1, n2, n3;
    dos_int i, x;
    if (b < 3) { x = b * 36 + 28; y = a * 28 + 43; }
    else { b -= 3; x = b * 32 + 140; y = a * 24 + 54; }
    w = 32; h = 24;
    old = cur_color_index_get();
    n1 = c ? 2 : 0;
    n2 = d ? 5 : 0;
    n3 = d ? 2 : 0;
    if (display_mode == 2) gfx_color_select(3);
    else if (display_mode == 3) gfx_color_select(0);
    else gfx_color_select(4);
    for (i = 0; i < n1; i++) { w += 2; h += 2; rect_border_draw(--x, --y, w, h); }
    gfx_color_select(8);
    for (i = 0; i < n2; i++) gfx_vline(x + w + i, y + 2, h);
    for (i = 0; i < n3; i++) gfx_bar(x + 5, y + h + i, w);
    gfx_color_select(old);
    return 0;
}


/* ---- F_963E (original code at 0x963E) ---- */
dos_int puzzle_clear_cell(dos_int a, dos_int b)
{
    dos_int x;
    dos_int y;
    if (b < 3) { x = b * 36 + 26; y = a * 28 + 41; }
    else { b -= 3; x = (b << 5) + 138; y = a * 24 + 52; }
    gfx_box(x, y, 42, 30);
    return 0;
}


/* ---- F_969D (original code at 0x969D) ---- */
dos_int puzzle_check_solved(void)
{
    dos_int r, n;
    dos_int i, j;
    r = 1;
    switch (puzzle_grid[0][3].rot) {
    case 0:
        for (i = 0, n = 0; i < 4; i++)
            for (j = 3; j < 6; j++, n++)
                if (puzzle_grid[i][j].kind != n || puzzle_grid[i][j].rot != 0) r = 0;
        break;
    case 1:
        for (i = 0, n = 0; i < 4; i++)
            for (j = 5; j >= 3; j--, n++)
                if (puzzle_grid[i][j].kind != n || puzzle_grid[i][j].rot != 1) r = 0;
        break;
    case 2:
        for (i = 3, n = 0; i >= 0; i--)
            for (j = 5; j >= 3; j--, n++)
                if (puzzle_grid[i][j].kind != n || puzzle_grid[i][j].rot != 2) r = 0;
        break;
    case 3:
        for (i = 3, n = 0; i >= 0; i--)
            for (j = 3; j < 6; j++, n++)
                if (puzzle_grid[i][j].kind != n || puzzle_grid[i][j].rot != 3) r = 0;
        break;
    }
    return r;
}
