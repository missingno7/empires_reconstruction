/* options.c -- portable port of src/OPTIONS.C: option/music/sound toggles
 * and the two record listings.
 */
#include "game.h"

/* RESOLVED: game_data.h now declares `extern struct dialog
 * dialog_toggle_music;` (DS:2160, 20 bytes) -- the earlier 160-byte
 * `struct dialog_toggle_music_record_s[8]` misattribution (an unrelated
 * catalog picked up by a name-text-match heuristic) is gone.
 * dialog_toggle_option, dialog_toggle_sound, dialog_slot_backup_list and
 * dialog_slot_list still have no generated symbol at all and are declared
 * locally below (harmless for a standalone, non-linked compile-check). */
extern struct dialog dialog_toggle_option;
extern struct dialog dialog_toggle_sound;
extern struct dialog dialog_slot_backup_list;
extern struct dialog dialog_slot_list;

dos_int options_toggle_option(void)
{
    dos_int rc;
    dialog_toggle_option.initial = (dos_uchar)!slot_table[current_slot].option;
    rc = dialog_run(&dialog_toggle_option);
    if (rc >= 0) {
        slot_table[current_slot].option = rc;
        if (rc) slot_table[current_slot].pending = -1;
    }
    return 0;
}

dos_int options_toggle_music(void)
{
    dos_int rc;
    dialog_toggle_music.initial = (dos_uchar)!slot_table[current_slot].music;
    rc = dialog_run(&dialog_toggle_music);
    if (rc >= 0) {
        music_enabled = slot_table[current_slot].music = rc;
        if (!rc) sound_voices_reset();
        else music_resume_if_valid();
    }
    return 0;
}

dos_int options_toggle_sound(void)
{
    dos_int rc;
    dialog_toggle_sound.initial = (dos_uchar)!slot_table[current_slot].sound;
    rc = dialog_run(&dialog_toggle_sound);
    if (rc >= 0) {
        sound_enabled = slot_table[current_slot].sound = rc;
        if (!rc) sound_stop_reset();
    }
    return 0;
}

dos_int slot_backup_list_show(void)
{
    dos_int height;
    dos_int i, y;

    dialog_draw(&dialog_slot_backup_list, 1);
    gfx_color_select(0);
    gfx_bar(43, 50, 39); gfx_bar(157, 50, 38); gfx_bar(223, 50, 35);
    height = dialog_line_height_get();
    for (i = 0, y = 0; i < 10; i++, y += height + 1)
        slot_row_draw(&slot_transfer_table[i], y + 52, 0);
    gfx_box(8, 0, 304, 200);
    do { i = keyboard_read_blocking_hotkeys(); } while (i != 27 && i != 13);
    dialog_restore_screen();
    return 0;
}

dos_int slot_list_show(void)
{
    dos_int height;
    dos_int i, y;

    dialog_draw(&dialog_slot_list, 1);
    gfx_color_select(0);
    gfx_bar(43, 50, 39); gfx_bar(153, 50, 38); gfx_bar(220, 50, 55);
    height = dialog_line_height_get();
    for (i = 0, y = 0; i < 10; i++, y += height + 1)
        slot_row_draw(&slot_table[i], y + 52, 1);
    gfx_box(8, 0, 304, 200);
    do { i = keyboard_read_blocking_hotkeys(); } while (i != 27 && i != 13);
    dialog_restore_screen();
    return 0;
}
