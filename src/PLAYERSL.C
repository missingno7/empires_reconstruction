/* src/PLAYERSL.C: Player-select screen.
   One translation unit; the sections below were the separate member
   sources of grouped module C_CDDD_D344 and keep their original ids. */

#include "DIALOG.H"
#include "C470.H"
#include "LAYOUT.H"
#include "VIDEO.H"

#define MK_FP(seg, ofs) ((void far *) (((unsigned long) (seg) << 16) | (unsigned) (ofs)))
#define FP_SEG(fp) ((unsigned) ((unsigned long) (void far *) (fp) >> 16))

extern void longjmp();
extern struct dialog dialog_select_quit_confirm;
extern char game_abort_jmpbuf[];
extern void slot_table_save();
extern struct dialog dialog_select_menu_confirm;
extern void sound_start(void), sound_stop_reset(), sound_voices_reset(), sound_request_count_dec();
extern struct dialog dialog_select_restart_confirm;
extern int music_enabled, sound_enabled;
extern int current_slot, gc5b0, gc5b2[];
extern int g22e0[], g22e8[];
extern char far *ui_gfx_shadow_a;
extern int resource_load_record(), player_select_draw_portraits();
extern void gfx_color_select(int n);
extern int g96, g94, g98, g9a;
extern char slot_flags_get();
extern int player_select_index;
extern int keyboard_read_blocking_hotkeys();
extern void keyboard_buffer_drain(void);
extern void player_select_restart_confirm();
extern void player_select_draw_highlight(void), player_select_clear_highlight(void);
extern int g22b2[];
extern int keyboard_chain_active(), player_select_draw_screen(), hud_prompt_select_draw(), player_select_choose_slot(), hud_panel_clear(), player_select_close_wipe();
extern void keyboard_chain_enable(void);
extern void resource_record_cache_reset();
extern void keyboard_chain_disable(void);
extern void player_select_load_flags(void), player_select_mark(int), menu_list_source_set_players(void), menu_list_source_set_default(void);
extern int g1776;
extern char g22d2[];
extern char far *farmalloc();
extern char far *ui_gfx_blob;                 /* DS:C5CA offset, DS:C5CC segment */
extern char far *ui_gfx_shadow_b;                 /* DS:C5BE offset, DS:C5C0 segment */

/* ---- F_CDDD (original code at 0xCDDD) ---- */
void player_select_quit_confirm(void)
{
    if (dialog_run(&dialog_select_quit_confirm) == 1)
        longjmp(game_abort_jmpbuf, 2);
    return 0;
}


/* ---- F_CE00 (original code at 0xCE00) ---- */
void player_select_menu_confirm(void)
{
    register int si;
    si = dialog_run(&dialog_select_menu_confirm);
    if (si == 1) {
        slot_table_save();
        longjmp(game_abort_jmpbuf, 1);
    }
    return 0;
}


/* ---- F_CE2A (original code at 0xCE2A) ---- */
void player_select_restart_confirm(void)
{
    register int si;
    sound_start();
    si = dialog_run(&dialog_select_restart_confirm);
    if (si == 1) {
        slot_table_save();
        sound_enabled = music_enabled = 0;
        sound_stop_reset();
        sound_voices_reset();
        longjmp(game_abort_jmpbuf, 3);
    }
    sound_request_count_dec();
    return 0;
}


/* ---- F_CE68 (original code at 0xCE68) ---- */
void player_select_mark(int i) { if(i!=-1) { gc5b0++; slot_table[current_slot].flags |= (gc5b2[i]=1<<i); } }


/* ---- F_CE9E (original code at 0xCE9E) ---- */
player_select_draw_portraits()
{
    register int i, x;
    int y;

    for (i = 0; i < 4; i++) {
        x = g22e0[i];
        y = g22e8[i];
        gfx_wipe_rect(x, y, 42, 32, i * 42, 400);
        gfx_copy_rect(x, y, ui_gfx_shadow_a, 0);
        gfx_wipe_rect(x, y, 42, 32, i * 42, 432);
        gfx_wipe_rect(i * 42, 400, 42, 32, x, y);
    }
}


/* ---- F_CF3C (original code at 0xCF3C) ---- */
player_select_draw_screen()
{
    register int i, n;

    g96 = 400;
    resource_load_record(26);
    gfx_blit_bitmap(0, 212, ui_gfx_shadow_a);
    gfx_wipe_rect(0, 200, 320, 200, 0, 0);
    resource_load_record(27);
    g94 = 0;
    gfx_copy_rect(46, 12, ui_gfx_shadow_a, 0);
    gfx_copy_rect(262, 12, ui_gfx_shadow_a, 0);
    g94 = 16;
    resource_load_record(28);
    g98 = 3;
    g9a = 156;
    gfx_copy_rect(6, 17, ui_gfx_shadow_a, 0);
    g98 = 4;
    g9a = 155;
    for (i = 0; i < 4; i++) {
        if (gc5b2[i])
            n = 1;
        else
            n = 0;
        n <<= 2;
        n += i;
        n += 3;
        resource_load_record(n + 26);
        gfx_copy_rect(g22e0[i], g22e8[i], ui_gfx_shadow_a, 0);
    }
    resource_load_record(37);
    gfx_color_select(0);
    gfx_clear_rect(0, 188, 320, 12);
    player_select_draw_portraits();
    gfx_box(0, 0, 320, 200);
}


/* ---- F_D089 (original code at 0xD089) ---- */
void player_select_load_flags(void) { register int flags,i; flags=slot_flags_get(); gc5b2[0]=flags&1; gc5b2[1]=flags&2; gc5b2[2]=flags&4; gc5b2[3]=flags&8; i=0; gc5b0=i; for(;i<4;i++) if(gc5b2[i]) gc5b0++; }


/* ---- F_D0D1 (original code at 0xD0D1) ---- */
void player_select_clear_highlight(void) { register int x,y; x=g22e0[player_select_index]; y=g22e8[player_select_index]; gfx_wipe_rect(player_select_index*42,400,42,32,x,y); gfx_box(x,y,42,32); }


/* ---- F_D117 (original code at 0xD117) ---- */
void player_select_draw_highlight(void) { register int x,y; x=g22e0[player_select_index]; y=g22e8[player_select_index]; gfx_wipe_rect(player_select_index*42,432,42,32,x,y); gfx_box(x,y,42,32); }


/* ---- F_D15D (original code at 0xD15D) ---- */
player_select_choose_slot()
{
    register int i, d;

    player_select_draw_highlight();
    while (1) {
        keyboard_buffer_drain();
        d = 0;
        switch (keyboard_read_blocking_hotkeys()) {
        case 0x14d:
        case 0x150:
            d++;
        case 0x148:
        case 0x14b:
            i = player_select_index;
            do {
                i = g22b2[i * 4 + d];
            } while (gc5b2[i]);
            if (i != player_select_index) {
                player_select_clear_highlight();
                player_select_index = i;
                player_select_draw_highlight();
            }
            break;
        case 13:
            player_select_clear_highlight();
            return;
        case 27:
            player_select_restart_confirm();
            break;
        }
    }
}


/* ---- F_D1D8 (original code at 0xD1D8) ---- */
player_select_close_wipe()
{
    register int i;

    for (i = 0x98; i > 0; i--) {
        gfx_wipe_rect(6, 13, 0x134, i, 6, 12);
        gfx_wipe_rect(6, i + 0xd4, 0x134, 1, 6, i + 12);
        gfx_box(6, 12, 0x134, i + 1);
    }
    gfx_wipe_rect(6, 0xd4, 0x134, 1, 6, 12);
    gfx_box(6, 12, 0x134, 1);
}


/* ---- F_D26C (original code at 0xD26C) ---- */
player_select_run(a)
int a;
{
    register int s;

    s = keyboard_chain_active();
    player_select_load_flags();
    resource_record_cache_reset(49);
    player_select_mark(a);
    player_select_draw_screen();
    menu_list_source_set_players();
    g1776 = 1;
    sound_request_count_dec();
    gfx_wipe_rect(0, 0, 320, 16, 0, 200);
    gfx_color_select(0);
    gfx_clear_rect(0, 0x184, 320, 12);
    slot_table[current_slot].resume_round = 0;
    keyboard_chain_enable();
    if (gc5b0 != 4) {
        hud_prompt_select_draw(g22d2);
        for (player_select_index = 0; player_select_index < 4; player_select_index++) {
            if (!gc5b2[player_select_index])
                break;
        }
        player_select_choose_slot();
    } else {
        hud_panel_clear();
        player_select_close_wipe();
        player_select_index = 4;
    }
    menu_list_source_set_default();
    if (!s)
        keyboard_chain_disable();
    g1776 = 0;
    sound_voices_reset();
    return player_select_index;
}


/* ---- F_D344 (original code at 0xD344) ---- */
/* F_D344 -- carve three far blocks out of one 64,128-byte allocation, each at
   a paragraph boundary derived from the first segment plus one.  The size is
   an unsigned long CONSTANT (`xor dx,dx`, not `cwd` -- the negative of rule
   16's signed-int case). */

void ui_gfx_alloc()
{
    register unsigned s;

    ui_gfx_blob = farmalloc(0xfa80L);
    s = FP_SEG(ui_gfx_blob) + 1;
    ui_gfx_blob = MK_FP(s, 0x0e);
    ui_gfx_shadow_a = MK_FP(s + 1, 0);
    ui_gfx_shadow_b = MK_FP(s + 0x7d4, 0);
}
