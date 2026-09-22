/* src/PLAYERSL.C: Player-select screen. */
#include "game.h"
#include "trace.h"

/* PORT: ui_gfx_alloc (F_D344, original src/PLAYERSL.C:254-263) is omitted
   from this file per the brief: its three-way carve of one 0xFA80-byte
   farmalloc block into ui_gfx_blob/ui_gfx_shadow_a/ui_gfx_shadow_b is
   already provided by portable/resource's resource_staging_init()
   (portable/include/resource.h: "Allocate the staging block (src/PLAYERSL.C
   ui_gfx_alloc)."), which keeps the same historical aliasing (ui_gfx_blob =
   block+0x0E, ui_gfx_shadow_a = ui_gfx_blob+2, ui_gfx_shadow_b =
   ui_gfx_shadow_a+0x7D30). */

/* ---- F_CDDD (original code at 0xCDDD) ---- */
void player_select_quit_confirm(void)
{
    /* PORT: the historical body is `if (...) longjmp(...); return 0;` in a
       function declared `void` -- Turbo C tolerated a value-return from a
       void function; C17 does not.  game_funcs.h agrees this is void, so
       the discarded "return 0" becomes a bare return. */
    if (dialog_run(&dialog_select_quit_confirm) == 1)
        game_abort(GAME_RETURN_MAP);
    return;
}


/* ---- F_CE00 (original code at 0xCE00) ---- */
void player_select_menu_confirm(void)
{
    dos_int si;
    si = dialog_run(&dialog_select_menu_confirm);
    if (si == 1) {
        slot_table_save();
        game_abort(GAME_RESTART);
    }
    return;
}


/* ---- F_CE2A (original code at 0xCE2A) ---- */
void player_select_restart_confirm(void)
{
    dos_int si;
    sound_start();
    si = dialog_run(&dialog_select_restart_confirm);
    if (si == 1) {
        slot_table_save();
        sound_enabled = music_enabled = 0;
        sound_stop_reset();
        sound_voices_reset();
        game_abort(GAME_EXIT);
    }
    sound_request_count_dec();
    return;
}


/* ---- F_CE68 (original code at 0xCE68) ---- */
void player_select_mark(dos_int i) { if (i != -1) { gc5b0++; slot_table[current_slot].flags |= (dos_char)(gc5b2[i] = 1 << i); } }


/* ---- F_CE9E (original code at 0xCE9E) ---- */
dos_int player_select_draw_portraits(void)
{
    dos_int i, x;
    dos_int y;

    for (i = 0; i < 4; i++) {
        x = g22e0[i];
        y = g22e8[i];
        gfx_wipe_rect(x, y, 42, 32, i * 42, 400);
        gfx_copy_rect(x, y, ui_gfx_shadow_a, 0);
        gfx_wipe_rect(x, y, 42, 32, i * 42, 432);
        gfx_wipe_rect(i * 42, 400, 42, 32, x, y);
    }
    return 0;
}


/* ---- F_CF3C (original code at 0xCF3C) ---- */
dos_int player_select_draw_screen(void)
{
    dos_int i, n;

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
    return 0;
}


/* ---- F_D089 (original code at 0xD089) ---- */
void player_select_load_flags(void)
{
    dos_int flags, i;
    flags = slot_flags_get();
    gc5b2[0] = flags & 1;
    gc5b2[1] = flags & 2;
    gc5b2[2] = flags & 4;
    gc5b2[3] = flags & 8;
    i = 0;
    gc5b0 = i;
    for (; i < 4; i++)
        if (gc5b2[i]) gc5b0++;
}


/* ---- F_D0D1 (original code at 0xD0D1) ---- */
void player_select_clear_highlight(void) { dos_int x, y; x = g22e0[player_select_index]; y = g22e8[player_select_index]; gfx_wipe_rect(player_select_index * 42, 400, 42, 32, x, y); gfx_box(x, y, 42, 32); }


/* ---- F_D117 (original code at 0xD117) ---- */
void player_select_draw_highlight(void) { dos_int x, y; x = g22e0[player_select_index]; y = g22e8[player_select_index]; gfx_wipe_rect(player_select_index * 42, 432, 42, 32, x, y); gfx_box(x, y, 42, 32); }


/* ---- F_D15D (original code at 0xD15D) ---- */
dos_int player_select_choose_slot(void)
{
    dos_int i, d;

    player_select_draw_highlight();
    while (1) {
        keyboard_buffer_drain();
        d = 0;
        switch (keyboard_read_blocking_hotkeys()) {
        case 0x14d:
        case 0x150:
            d++;
            /* fallthrough */
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
            return 0;
        case 27:
            player_select_restart_confirm();
            break;
        }
    }
}


/* ---- F_D1D8 (original code at 0xD1D8) ---- */
dos_int player_select_close_wipe(void)
{
    dos_int i;

    for (i = 0x98; i > 0; i--) {
        gfx_wipe_rect(6, 13, 0x134, i, 6, 12);
        gfx_wipe_rect(6, i + 0xd4, 0x134, 1, 6, i + 12);
        gfx_box(6, 12, 0x134, i + 1);
    }
    gfx_wipe_rect(6, 0xd4, 0x134, 1, 6, 12);
    gfx_box(6, 12, 0x134, 1);
    return 0;
}


/* ---- F_D26C (original code at 0xD26C) ---- */
dos_int player_select_run(dos_int a)
{
    EMPIRES_TRACE("player_select_run(%d)", a);
    dos_int s;

    s = keyboard_chain_active();
    player_select_load_flags();
    resource_record_cache_reset(49);
    player_select_mark(a);
    player_select_draw_screen();
    menu_list_source_set_players();
    snd_flag2 = 1;
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
    snd_flag2 = 0;
    sound_voices_reset();
    return player_select_index;
}
