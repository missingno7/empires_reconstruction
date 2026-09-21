/* prompts.c -- portable port of src/PROMPTS.C: prompt boxes (continue,
 * select, confirm).
 * One translation unit; the sections below were the separate member
 * sources of grouped module C_75F3_7856 and keep their original ids.
 */
#include "game.h"

/* Ported-C-owned DATA (docs/portable/state-map.md: DATA_0107B0_TABLE at
 * DS:0B80 = gb80+energy_meter+hud_prompt_kind+gb85's low byte,
 * DATA_0107B6_PAD at DS:0B86 = gb85's high byte; both code_owner F_75F3).
 * Referenced (extern) by the not-yet-ported src/HUD.C/src/HUDPMSG.C; no
 * generated header declares any of the four, so those files keep local
 * `extern` declarations for them until they land in game_state.h. */
dos_int  gb80 = 4;
dos_char energy_meter = 4;
dos_int  hud_prompt_kind = 0;
dos_int  gb85 = 0;

/* ---- F_75F3 (original code at 0x75F3) ---- */
void hud_prompt_continue_draw(void)
{
    dos_char cap[15] = "\027\030 to Continue";      /* C_DATA_75F3 (ported-C-owned) */
    dos_int s, d;

    hud_prompt_kind = 2;
    s = cur_color_index_get();
    d = sprite_sheet_index_get();
    if (display_mode == 2)
        gfx_color_select(5);
    else
        gfx_color_select(0xf);
    sprite_sheet_select(0);
    text_draw_wrapped(0x18, 0xbc, cap);
    gfx_box(0x18, 0xbc, 0x94, 0xa);
    gfx_color_select(s);
    sprite_sheet_select(d);
}

/* ---- F_7676 (original code at 0x7676) ---- */
void hud_prompt_continue_clear(void)
{
    anim_step_loop(0x18, 0x184, 0x94, 10, 0x18, 0xbc);
}

/* ---- F_7695 (original code at 0x7695) ---- */
dos_int hud_prompt_select_draw(dos_char *p)
{
    dos_char cap[13] = "\027\030 to Select";        /* C_DATA_7695 (ported-C-owned) */
    dos_int s, d;

    s = cur_color_index_get();
    d = sprite_sheet_index_get();
    hud_prompt_kind = 3;
    gc0f6 = p;
    gc0fc = 0;
    gfx_color_select(0);
    gfx_clear_rect(0, 0xbc, 0x140, 12);
    gfx_color_select(15);
    sprite_sheet_select(0);
    text_draw_wrapped(10, 0xbd, p);
    text_draw_wrapped(0xaa, 0xbd, cap);
    gfx_box(0, 0xbc, 0x140, 12);
    gfx_color_select(s);
    sprite_sheet_select(d);
    return 0;   /* PORT: original had no return statement (K&R implicit int) */
}

/* ---- F_7747 (original code at 0x7747) ---- */
void hud_prompt_message_run(dos_char *p)
{
    dos_int key, saved;

    saved = menu_list_active();
    menu_list_disable();
    hud_prompt_confirm_draw(p, 0, 15, 1, 0);
    do {
        key = keyboard_read_blocking_hotkeys();
    } while (key != 13 && key != 27);
    if (saved) menu_list_enable();
    hud_panel_open();
}

/* ---- F_778B (original code at 0x778B) ---- */
dos_int hud_prompt_confirm_draw(dos_char *p, dos_int a, dos_int b, dos_int c, dos_int e)
{
    dos_char cap[15] = "\027\030 to Continue";      /* C_DATA_778B (ported-C-owned) */
    dos_int s, d;

    s = cur_color_index_get();
    d = sprite_sheet_index_get();
    hud_prompt_kind = 4;
    gc0fc = b;
    gfx_color_select(0);
    rect_border_draw(6, 0xa2, 0x134, 0x24);
    gfx_color_select(b);
    gfx_clear_rect(8, 0xa3, 0x130, 0x22);
    gfx_color_select(a);
    sprite_sheet_select(c);
    text_draw_wrapped(12, 0xa5, p);
    if (e)
        text_draw_wrapped(0xaa, 0xb9, cap);
    gfx_box(6, 0xa2, 0x134, 0x24);
    gfx_color_select(s);
    sprite_sheet_select(d);
    return 0;   /* PORT: original had no return statement (K&R implicit int) */
}
