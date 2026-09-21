/* slotmenu.c -- portable port of src/SLOTMENU.C: save-slot menus (list,
 * name entry, player type, confirmations).
 * One translation unit; the sections below were the separate member
 * sources of grouped module C_A33F_AD0E and keep their original ids.
 *
 * state-map.md's porting note: SLOTMENU.C uses the scalar spelling of
 * g13ef (LEVEL.C's `int g13ef[]` is generated as `dos_int g13ef[1]`); this
 * file spells it g13ef[0] throughout.
 *
 * `box`/`bar`/`fill` (F_039F/F_03A2/F_03AB) are, per
 * docs/portable/funcs-inventory.md sec 3, the SAME asm routines as
 * gfx_box/gfx_bar/gfx_fill_rect under an alternate PUBLIC label; ported as
 * direct calls to the gfx.h names.
 */
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
/* strcpy below matches the historical
                                     call exactly; length is bounded by
                                     the 9-byte slot_table[].text field
                                     the same way the historical far-
                                     pointer strcpy was. */
#include "game.h"

/* music_track_handle (DS:237E) is ported-C-owned DATA defined by
 * portable/game/rescache.c; no generated header declares it (see that
 * file's own comment), so this file keeps a local extern. */
extern dos_int music_track_handle;

/* ---- F_A33F (original code at 0xA33F) ---- */
dos_int slot_list_draw(void)
{
    dos_int i, y;

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
    return 0;   /* PORT: original had no return statement (K&R implicit int) */
}

/* ---- F_A40A (original code at 0xA40A) ---- */
void slot_cursor_box(dos_int x, dos_int y, dos_int c)
{
    gfx_color_select(c);
    gfx_clear_rect(x, y, 8, 10);
    gfx_box(x, y, 8, 10);
}

/* ---- F_A43C (original code at 0xA43C) ---- */
dos_int slot_input_wait_key(dos_int a, dos_int b)
{
    dos_int t, c;

    c = 15;
    t = 0;
    timer_deadline_arm(23);
    while (!keyboard_poll_nonblocking()) {
        if (timer_deadline_reached()) {
            if ((t = !t))
                slot_cursor_box(a, b, c ^= 15);
            if (++g13ef[0] >= 3)
                g13ef[0] = 0;
            gfx_wipe_rect(g13ef[0] * 18, 400, 18, 33, 4, 85);
            gfx_copy_rect_flip_h(4, 85, 18, 33, 0x12a, 85);
            gfx_box(4, 85, 18, 33);
            gfx_box(0x12a, 85, 18, 33);
            timer_deadline_arm(23);
        }
    }
    slot_cursor_box(a, b, 15);
    return keyboard_read_blocking_hotkeys();
}

/* ---- F_A525 (original code at 0xA525) ----
 * RESOLVED: `_ctype[c+1] & 14` is Turbo C 2.0's CTYPE.H classification
 * table, whose bit assignments are `_IS_UP 1, _IS_LOW 2, _IS_DIG 4,
 * _IS_SP 8` (docs/historical-runtime-publics.md's "alnum filter via
 * _ctype" description was imprecise: mask 14 = 2|4|8 = lower|digit|space,
 * NOT upper -- so a directly-typed uppercase letter (e.g. via Shift) is
 * rejected here, matching the BIOS keyboard driver's plain lowercase
 * default and this function's own `if(!i) c=toupper(c)` auto-capitalize
 * of only the first character).  The `case 32:` arm above already
 * consumes space before this default branch runs, so the `_IS_SP` bit is
 * inert here in practice; kept in the predicate for literal fidelity to
 * the historical mask.  cclib.h does not expose Turbo C's raw _ctype
 * table (tu-porting-rules.md sec 4 keeps only the documented CC.LIB call
 * list), so this uses <ctype.h>'s islower()/isdigit()/isspace(), which
 * reproduce the three classification bits exactly.
 */
dos_int player_name_edit(void)
{
    dos_int c, y;
    dos_int i, x;

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
        g12d0[i] = (dos_char)c;
        switch (c = slot_input_wait_key(x + 1, y)) {
        case 13:
            if (i) {
                g12d0[i] = 0;
                return 0;
            }
            /* fallthrough */
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
            if (c < 256 && (islower((unsigned char)c) || isdigit((unsigned char)c) || isspace((unsigned char)c)) && i < 8) {
                if (!i)
                    c = toupper(c);
                g12d0[i++] = (dos_char)c;
            }
        }
    }
}

/* ---- F_A658 (original code at 0xA658) ----
 * The historical argument aliases record 5's zero word, not local storage.
 */
dos_int player_type_select(void)
{
    /* menu_empty_record (DS:13C5) is now generated (game_data.h); no
     * local extern needed. */
    dos_int sel, quit;
    dos_int sv2, sv1;

    sel = 0x10;
    quit = 0;
    sv2 = keyboard_chain_active();
    keyboard_chain_enable(); keyboard_buffer_drain(); ui_overlay_show(); sound_start();
    sv1 = menu_list_active();
    menu_list_disable();
    gfx_box(0, 0, 0x140, 0xc8);
    dialog_draw(&menu_empty_record, 1);
    gfx_color_select(0);
    gfx_bar(0x26, 0x73, 0xf4);
    gfx_box(0x24, 0x73, 0xf6, 1);
    gfx_fill_rect(0x28, 0x7e, 0xee, 0xa);
    gfx_box(0x28, 0x7e, 0xee, 0x14);
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
dos_int confirm_quit_dialog(void)
{
    dos_int a, b;
    dos_int i, di;

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

/* ---- F_A85E (original code at 0xA85E) ----
 * PORT: `tbl`/struct tbl_entry (include/TBL.H) is a byte-identical
 * alternate view of slot_table/struct c470_record at the same 27-byte
 * stride and offsets (game_structs.h's own comment on tbl_entry cites the
 * matching offsets); per tu-porting-rules.md ("use the generated one and
 * adapt indexing without changing semantics") this writes through
 * slot_table[]'s named c470_record fields instead of tbl_entry's, with a
 * field-name cross-reference in comments: b11->flags, l21->state,
 * a9->value, d13->sound, f15->music, h17->option, j19->pending.
 */
dos_int player_slot_add_run(void)
{
    dos_int rc, flag;
    dos_int sv;

    if (slot_used_count == 10) {
        dialog_run(&dialog_player_name_full);
        slot_select_error = 2;
        return -2;
    }
    flag = keyboard_chain_active();
    keyboard_chain_enable(); keyboard_buffer_drain(); ui_overlay_show(); sound_start();
    sv = menu_list_active();
    menu_list_disable();
    dialog_draw(&dialog_player_name_entry, 1);
    gfx_color_select(0);
    rect_border_draw(0x58, 0x5f, 0x78, 0x13);
    gfx_box(0x58, 0x5f, 0x78, 0x13);
    rc = player_name_edit();
    dialog_restore_screen();
    if (sv) menu_list_enable();
    sound_request_count_dec(); ui_overlay_hide();
    if (!flag) keyboard_chain_disable();
    keyboard_buffer_drain();
    if (rc > -1) {
        rc = slot_used_count;
        setmem(&slot_table[rc], 0x1b, 0);
        if ((slot_table[rc].flags = (dos_char)player_type_select()) != 0) {
            slot_table[rc].state = 4;
            slot_table[rc].value = 1;
            slot_table[rc].sound = sound_enabled;
            slot_table[rc].music = music_enabled;
            if (slot_table[rc].flags == 0x10) slot_table[rc].option = 1;
            slot_table[rc].pending = -1;
            strcpy((char *)slot_table[rc].text, (char *)g12d0);
        } else {
            slot_select_error = 1; rc = -2;
        }
    } else if (slot_used_count != 0) { slot_select_error = 1; rc = -2; }
    return rc;
}

/* ---- F_AA1F (original code at 0xAA1F) ----
 * Exact Turbo C recovery. Local order and final switch-case fallthrough
 * reproduce the original frame and branch layout.
 */
dos_int slot_list_select_loop(void)
{
    dos_int old, del_result;
    dos_char buffer[200];
    dos_int selected, count;

    /* g13b8 is now `#define g13b8 (dialog_slot_delete_confirm.text)`
     * (game_data.h); no local extern needed. */

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
                str_concat_far_list(buffer, g12e5, (dos_char *)(slot_table + selected), g1356, (dos_char *)0);
                g13b8 = buffer;
                if ((del_result = dialog_run(&dialog_slot_delete_confirm)) >= 0) { slot_select_error = 1; if (del_result) slot_delete(selected); }
                return -2;
            }
            return selected;
        case 18:
            if (slot_select_error == 1) { slot_select_error = 2; slot_row_highlight(old); return -2; }
        }
        slot_row_highlight(old);
    }
}

/* ---- F_AB66 (original code at 0xAB66) ----
 * Exact Turbo C recovery of the 385-byte selection/workspace routine.
 */
dos_int slot_menu_run(void)
{
    dos_int y, saved;
    dos_int selected, i;

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
dos_int slot_is_new_game(void)
{
    return (slot_table[current_slot].flags & 0x20) == 0x20;
}

/* ---- F_AD0E (original code at 0xAD0E) ---- */
dos_char slot_flags_get(void)
{
    return slot_table[current_slot].flags;
}
