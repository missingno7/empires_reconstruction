/* src/SLOTMENU.C: Save-slot menus: list, name entry, player type and confirmations.
   One translation unit; the sections below were the separate member
   sources of grouped module C_A33F_AD0E and keep their original ids. */

#include "C470.H"
#include "TBL.H"
#include "DIALOG.H"
#include "VIDEO.H"

/* Declarations below are grouped per original section. Many sections
   redeclare the same external symbol in a different form (prototyped vs.
   K&R empty-parens, `(void)` vs `()`); those forms are kept local to the
   section that needs them rather than unified, since a stronger or weaker
   prototype visible at a call site can change the bytes Turbo C emits for
   the call (see e.g. F_A43C's weak `slot_cursor_box()` redeclaration and
   F_A85E's wide-prototype `setmem` versus F_A525's narrow one). Plain data
   symbols (ints, char arrays) have no such risk and are merged below. */
extern int current_slot;
extern int cur, err, sound_enabled, music_enabled;
extern char g12d0[];
extern void text_draw_wrapped(int, int, char far *);

/* ---- F_A33F (original code at 0xA33F) ---- */
extern int hud_prompt_select_draw(), hud_panel_clear();
extern void gfx_color_select();
extern char g12d9[], g1660[];

slot_list_draw()
{
    register int i, y;

    gfx_wipe_rect(0, 200, 320, 200, 0, 0);
    gfx_color_select(0);
    y = 49;
    for (i = 0; i < slot_used_count; i++)
        slot_row_draw(slot_table + i, y += 11, 1);
    if (slot_select_error) {
        hud_prompt_select_draw(g12d9);
        gfx_color_select(0);
        gfx_wipe_rect(0, 0x1b8 + (slot_select_error - 1) * 17, 192, 17, 66, 38);
        if (slot_select_error == 1)
            text_draw_wrapped(43, y + 11, g1660);
    } else if (!slot_used_count)
        hud_panel_clear();
    gfx_box(0, 0, 320, 200);
}


/* ---- F_A40A (original code at 0xA40A) ---- */
extern void gfx_color_select();

void slot_cursor_box(int x,int y,int c) { gfx_color_select(c); gfx_clear_rect(x,y,8,10); gfx_box(x,y,8,10); }


/* ---- F_A43C (original code at 0xA43C) ---- */
extern int timer_deadline_reached(), keyboard_poll_nonblocking(), keyboard_read_blocking_hotkeys();
extern void gfx_copy_rect_flip_h();
extern void timer_deadline_arm();
extern void slot_cursor_box();
extern int g13ef;

slot_input_wait_key(a,b)
int a, b;
{
    register int t, c;

    c = 15;
    t = 0;
    timer_deadline_arm(23);
    while (!keyboard_poll_nonblocking()) {
        if (timer_deadline_reached()) {
            if (t = !t)
                slot_cursor_box(a, b, c ^= 15);
            if (++g13ef >= 3)
                g13ef = 0;
            gfx_wipe_rect(g13ef * 18, 400, 18, 33, 4, 85);
            gfx_copy_rect_flip_h(4, 85, 18, 33, 0x12a, 85);
            gfx_box(4, 85, 18, 33);
            gfx_box(0x12a, 85, 18, 33);
            timer_deadline_arm(23);
        }
    }
    slot_cursor_box(a, b, 15);
    return keyboard_read_blocking_hotkeys();
}


/* ---- F_A525 (original code at 0xA525) ---- */
extern int text_line_width(), slot_input_wait_key(), toupper();
extern void setmem();
extern void gfx_color_select();
extern unsigned char _ctype[];

player_name_edit()
{
    int c, y;
    register int i, x;

    setmem(g12d0, 8, 95);
    y = 100;
    i = 0;
    while (1) {
        gfx_color_select(15);
        gfx_clear_rect(96, 100, 100, 10);
        gfx_color_select(0);
        text_draw_wrapped(96, 100, g12d0);
        gfx_box(96, 100, 100, 10);
        c = g12d0[i];
        g12d0[i] = 0;
        x = text_line_width(g12d0) + 97;
        g12d0[i] = c;
        switch (c = slot_input_wait_key(x + 1, y)) {
        case 13:
            if (i) {
                g12d0[i] = 0;
                return 0;
            }
        case 8:
            if (i > 0)
                g12d0[--i] = 95;
            break;
        case 27:
            g12d0[0] = 0;
            return -1;
        case 32:
            if (i && i < 8)
                g12d0[i++] = 32;
            break;
        default:
            if (c < 256 && (_ctype[c + 1] & 14) && i < 8) {
                if (!i)
                    c = toupper(c);
                g12d0[i++] = c;
            }
        }
    }
}


/* ---- F_A658 (original code at 0xA658) ---- */
/* F_A658 -- a menu loop: init calls, a sparse 4-case switch on a key code
   dispatched through a cs: table, and teardown. */
extern int  keyboard_chain_active(void);          /* 6B74 */
/*@SYM _keyboard_chain_active=0x6B74 kind=f key=functions/F_6B74.entry*/
extern void keyboard_chain_enable(void);          /* 6990 */
/*@SYM _keyboard_chain_enable=0x6990 kind=f key=functions/F_6990.entry*/
extern void keyboard_buffer_drain(void);          /* 6B66 */
/*@SYM _keyboard_buffer_drain=0x6B66 kind=f key=functions/F_6B66.entry*/
extern void ui_overlay_show(void);          /* 703E */
/*@SYM _ui_overlay_show=0x703E kind=f key=functions/F_703E.entry*/
extern void sound_start(void);          /* D593 */
/*@SYM _sound_start=0xD593 kind=f key=functions/F_D593.entry*/
extern int  menu_list_active(void);          /* 792C */
/*@SYM _menu_list_active=0x792C kind=f key=functions/F_792C.entry*/
extern void menu_list_disable(void);          /* 7925 */
/*@SYM _menu_list_disable=0x7925 kind=f key=functions/F_7925.entry*/
extern void box(int a, int b, int c, int d);        /* 039F */
/*@SYM _box=0x039F kind=f key=functions/F_039F.entry*/
extern void gfx_color_select(int a);                            /* 01CE */
/*@SYM _gfx_color_select=0x01CE kind=f key=functions/F_01CE.entry*/
extern void bar(int a, int b, int c);               /* 03A2 */
/*@SYM _bar=0x03A2 kind=f key=functions/F_03A2.entry*/
extern void fill(int a, int b, int c, int d);       /* 03AB */
/*@SYM _fill=0x03AB kind=f key=functions/F_03AB.entry*/
extern int  menu_wait_key_animated(void);                           /* AF45 */
/*@SYM _menu_wait_key_animated=0xAF45 kind=f key=functions/F_AF45.entry*/
extern void player_type_toggle_draw(void);                              /* A19D */
/*@SYM _player_type_toggle_draw=0xA19D kind=f key=functions/F_A19D.entry*/
extern void dialog_restore_screen(void);          /* 8453 */
/*@SYM _dialog_restore_screen=0x8453 kind=f key=functions/F_8453.entry*/
extern void menu_list_enable(void);          /* 791E */
/*@SYM _menu_list_enable=0x791E kind=f key=functions/F_791E.entry*/
extern void sound_request_count_dec(void);          /* D5A6 */
/*@SYM _sound_request_count_dec=0xD5A6 kind=f key=functions/F_D5A6.entry*/
extern void ui_overlay_hide(void);          /* 7162 */
/*@SYM _ui_overlay_hide=0x7162 kind=f key=functions/F_7162.entry*/
extern void keyboard_chain_disable(void);          /* 6997 */
/*@SYM _keyboard_chain_disable=0x6997 kind=f key=functions/F_6997.entry*/

/* The historical argument aliases record 5's zero word, not local storage. */
extern struct dialog near menu_empty_record;

int player_type_select(void)
{
    register int sel, quit;
    int sv2, sv1;

    sel = 0x10;
    quit = 0;
    sv2 = keyboard_chain_active();
    keyboard_chain_enable(); keyboard_buffer_drain(); ui_overlay_show(); sound_start();
    sv1 = menu_list_active();
    menu_list_disable();
    box(0, 0, 0x140, 0xc8);
    dialog_draw(&menu_empty_record, 1);
    gfx_color_select(0);
    bar(0x26, 0x73, 0xf4);
    box(0x24, 0x73, 0xf6, 1);
    fill(0x28, 0x7e, 0xee, 0xa);
    box(0x28, 0x7e, 0xee, 0x14);
    while (!quit) {
        switch (menu_wait_key_animated()) {
        case 0x1b:  quit = 1; sel = 0;     break;
        case 0x148:
        case 0x150: sel ^= 0x30; player_type_toggle_draw();    break;
        case 0x0d:  quit = sel;            break;
        }
    }
    dialog_restore_screen();
    if (sv1) menu_list_enable();
    sound_request_count_dec(); ui_overlay_hide();
    if (!sv2) keyboard_chain_disable();
    keyboard_buffer_drain();
    return sel;
}


/* ---- F_A768 (original code at 0xA768) ---- */
extern struct dialog dialog_quit_confirm;
extern int keyboard_chain_active(), menu_list_active(), keyboard_read_blocking_hotkeys();
extern void sound_start(void);
extern void menu_list_disable(void);
extern void keyboard_chain_enable(void);
extern void ui_overlay_hide(void);
extern void ui_overlay_show(void);
extern void menu_list_enable(void);
extern void dialog_restore_screen();
extern void gfx_color_select(int n);
extern void keyboard_chain_disable(void);
extern void sound_request_count_dec(void);
extern void quit_confirm_toggle_draw(void);

confirm_quit_dialog()
{
    int a, b;
    register int i, di;

    i = 0;
    di = -1;
    a = keyboard_chain_active();
    sound_start();
    b = menu_list_active();
    menu_list_disable();
    keyboard_chain_enable();
    ui_overlay_show();
    dialog_draw(&dialog_quit_confirm, 1);
    gfx_color_select(0);
    gfx_bar(50, 95, 218);
    gfx_box(50, 95, 218, 1);
    gfx_fill_rect(50, 103, 218, 10);
    gfx_box(50, 103, 218, 20);
    while (di < 0) {
        switch (keyboard_read_blocking_hotkeys()) {
        case 27: di = 1; i = 0; break;
        case 328:
        case 336: i ^= 1; quit_confirm_toggle_draw(); break;
        case 13: di = i; break;
        }
    }
    dialog_restore_screen();
    ui_overlay_hide();
    if (!a) keyboard_chain_disable();
    if (b) menu_list_enable();
    sound_request_count_dec();
    return i;
}


/* ---- F_A85E (original code at 0xA85E) ---- */
extern struct dialog dialog_player_name_full, dialog_player_name_entry;
extern int  keyboard_chain_active(void);                           /* 6B74 */
/*@SYM _keyboard_chain_active=0x6B74 kind=f key=functions/F_6B74.entry*/
extern void keyboard_chain_enable(void), keyboard_buffer_drain(void), ui_overlay_show(void), sound_start(void);
extern int  menu_list_active(void);                           /* 792C */
/*@SYM _menu_list_active=0x792C kind=f key=functions/F_792C.entry*/
extern void menu_list_disable(void);                           /* 7925 */
/*@SYM _menu_list_disable=0x7925 kind=f key=functions/F_7925.entry*/
extern void gfx_color_select(int a);                            /* 01CE */
/*@SYM _gfx_color_select=0x01CE kind=f key=functions/F_01CE.entry*/
extern void rect_border_draw(int a, int b, int c, int d);      /* 0355 */
/*@SYM _rect_border_draw=0x0355 kind=f key=functions/F_0355.entry*/
extern void box(int a, int b, int c, int d);        /* 039F */
/*@SYM _box=0x039F kind=f key=functions/F_039F.entry*/
extern int  player_name_edit(void);                             /* A525 */
/*@SYM _player_name_edit=0xA525 kind=f key=functions/F_A525.entry*/
extern void dialog_restore_screen(void), menu_list_enable(void), sound_request_count_dec(void), ui_overlay_hide(void);
extern void keyboard_chain_disable(void), keyboard_buffer_drain(void);
extern void setmem(struct tbl_entry far *p, unsigned n, int v); /* F304 */
/*@SYM _setmem=0xF304 kind=f key=functions/F_F304.entry*/
extern int  player_type_select(void);                            /* A658 */
extern char far *strcpy(struct tbl_entry far *p, char far *s);      /* F2DB */
/*@SYM _strcpy=0xF2DB kind=f key=functions/F_F2DB.entry*/

int player_slot_add_run(void)
{
    register int rc, flag;
    int sv;

    if (cur == 10) { dialog_run(&dialog_player_name_full); err = 2; return -2; }
    flag = keyboard_chain_active();
    keyboard_chain_enable(); keyboard_buffer_drain(); ui_overlay_show(); sound_start();
    sv = menu_list_active();
    menu_list_disable();
    dialog_draw(&dialog_player_name_entry, 1);
    gfx_color_select(0);
    rect_border_draw(0x58, 0x5f, 0x78, 0x13);
    box(0x58, 0x5f, 0x78, 0x13);
    rc = player_name_edit();
    dialog_restore_screen();
    if (sv) menu_list_enable();
    sound_request_count_dec(); ui_overlay_hide();
    if (!flag) keyboard_chain_disable();
    keyboard_buffer_drain();
    if (rc > -1) {
        rc = cur;
        setmem((struct tbl_entry far *)tbl + rc, 0x1b, 0);
        if ((tbl[rc].b11 = player_type_select()) != 0) {
            tbl[rc].l21 = 4;
            tbl[rc].a9  = 1;
            tbl[rc].d13 = sound_enabled;
            tbl[rc].f15 = music_enabled;
            if (tbl[rc].b11 == 0x10) tbl[rc].h17 = 1;
            tbl[rc].j19 = -1;
            strcpy(&tbl[rc], g12d0);
        } else {
            err = 1; rc = -2;
        }
    } else if (cur != 0) { err = 1; rc = -2; }
    return rc;
}


/* ---- F_AA1F (original code at 0xAA1F) ---- */
/* Exact Turbo C recovery. Local order and final switch-case fallthrough
 * reproduce the original frame and branch layout. */
extern char near g1356[],g12e5[];
extern struct dialog near dialog_slot_delete_confirm;
extern char far *g13b8;
extern int menu_wait_key_animated();
extern void keyboard_buffer_drain(void);
extern void slot_row_highlight(),slot_delete(int);
/* str_concat_far_list appends far strings until a null pointer; this call site needs the
   record argument typed as a far pointer for exact code. */
extern long str_concat_far_list(char far *dest,char far *a,struct c470_record far *b,char far *c,char far *end);

int slot_list_select_loop(void)
{
    int old, result;
    char buffer[200];
    register int selected, count;

    if (slot_select_error == 1) { selected = slot_used_count; count = selected + 1; } else { selected = 0; count = slot_used_count; }
    while (1) {
        slot_row_highlight(old = selected); keyboard_buffer_drain();
        switch (menu_wait_key_animated()) {
        case 0x150: ++selected; selected %= count; break;
        case 0x148: --selected; selected = (selected + count) % count; break;
        case 27:
            slot_row_highlight(old); if (slot_select_error == 1) return -1;
            slot_select_error = 1; return -2;
        case 13:
            slot_row_highlight(old);
            if (selected == slot_used_count) { slot_select_error = 0; return -2; }
            else if (slot_select_error == 2) {
                str_concat_far_list(buffer, g12e5, slot_table + selected, g1356, (char far *)0);
                g13b8 = buffer;
                if ((result = dialog_run(&dialog_slot_delete_confirm)) >= 0) { slot_select_error = 1; if (result) slot_delete(selected); }
                return -2;
            }
            return selected;
        case 18:
            if (slot_select_error == 1) { slot_select_error = 2; slot_row_highlight(old); return -2; }
        }
        slot_row_highlight(old);
    }
}


/* ---- F_AB66 (original code at 0xAB66) ---- */
/* Exact Turbo C recovery of the 385-byte selection/workspace routine.
   The far-pointer slot_row_draw prototype and local declaration order are byte-significant. */
extern int keyboard_chain_active(), slot_menu_draw_header(), slot_find_free(), slot_list_draw(), slot_list_select_loop(), player_slot_add_run();
extern void menu_list_disable(void);
extern void sound_start(void);
extern void keyboard_chain_enable(void);
extern void keyboard_buffer_drain(void);
extern void sound_voices_reset();
extern void sound_stop_reset(void);
extern void gfx_color_select(int n);
extern void keyboard_chain_disable(void);
extern void sound_request_count_dec(void);
extern void anim_step_loop(int,int,int,int,int,int);
extern void slot_table_save();
extern int music_track_handle,g98,g9a;
extern struct dialog near g139d;
extern char far *ui_gfx_shadow_a;

int slot_menu_run(void)
{
    int y, saved;
    register int selected, i;

    saved = keyboard_chain_active(); menu_list_disable(); sound_stop_reset(); sound_voices_reset();
    music_track_handle = -1; sound_start(); g98 = 0; g9a = 159; slot_menu_draw_header();
    gfx_blit_bitmap(0, 200, ui_gfx_shadow_a); gfx_color_select(0); slot_used_count = slot_find_free();
    y = 249;
    for (i = 0; i < slot_used_count; i++) slot_row_draw(slot_table + i, y += 11, 1);
    anim_step_loop(0, 200, 320, 200, 0, 0);
    gfx_blit_bitmap(0, 200, ui_gfx_shadow_a); slot_select_error = 1; keyboard_chain_enable();
    do {
        if (!(slot_used_count = slot_find_free())) slot_select_error = 0;
        keyboard_buffer_drain(); slot_list_draw();
        if (slot_select_error) selected = slot_list_select_loop(); else selected = player_slot_add_run();
        if (selected == -1 && dialog_run(&g139d) != 1) selected = -2;
    } while (selected < -1);
    slot_table_save(); if (!saved) keyboard_chain_disable(); gfx_color_select(1);
    gfx_clear_rect(0, 0, 320, 200); gfx_box(0, 0, 320, 200);
    g98 = 4; g9a = 155;
    sound_enabled = slot_table[selected].sound;
    music_enabled = slot_table[selected].music;
    sound_request_count_dec(); return current_slot = selected;
}


/* ---- F_ACE7 (original code at 0xACE7) ---- */
int face7(void) { return (slot_table[current_slot].flags & 0x20) == 0x20; }


/* ---- F_AD0E (original code at 0xAD0E) ---- */
char slot_flags_get(void) { return slot_table[current_slot].flags; }
