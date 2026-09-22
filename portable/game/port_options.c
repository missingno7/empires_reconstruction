/* port_options.c -- portable F3 Options overlay and native-looking dialogs.
 *
 * The generated menu records are mutable linker objects, so install the
 * portable six-row view at runtime in both historical menu catalogs.  This
 * preserves the DATA source and keeps these host-only rows outside the
 * generated/regenerated historical tree.
 */
#include "game.h"

#include "port_options.h"
#include "port_settings.h"

#include <stdio.h>
#include <string.h>

dos_char port_options_menu_text[] =
    "New User Messages\nMusic\nMusic Volume\nSound Effects\n"
    "Sound Effects Volume\nInterpolation\0";

dos_int port_options_music_volume(void);
dos_int port_options_sound_volume(void);
dos_int port_options_interpolation(void);

void (*port_options_menu_callbacks[PORT_OPTIONS_ROW_COUNT])(void) = {
    (void (*)(void))options_toggle_option,
    (void (*)(void))options_toggle_music,
    (void (*)(void))port_options_music_volume,
    (void (*)(void))options_toggle_sound,
    (void (*)(void))port_options_sound_volume,
    (void (*)(void))port_options_interpolation
};

void port_options_install(void)
{
    struct menu_record *records[] = { &menu_records_0CFA[2], &menu_records_0D3C[2] };
    for (size_t i = 0; i < sizeof records / sizeof records[0]; i++) {
        records[i]->count = PORT_OPTIONS_ROW_COUNT;
        records[i]->text = port_options_menu_text;
        records[i]->callbacks = port_options_menu_callbacks;
        /* dialog_layout adds its ten-pixel frame around this content width;
         * x=136, width=170 therefore stays inside the 320px screen. */
        records[i]->width = 170;
    }
}

static void volume_text(dos_char *text, size_t n, const char *name, int value)
{
    snprintf((char *)text, n, "Use Left/Right to change\nEnter to keep, Esc to cancel\n%s: %d%%",
             name, value);
}

static int volume_dialog(const char *title, const char *name, int original, bool music)
{
    struct dialog d;
    dos_char text[128];
    dos_int outer, menu_state, key;
    int value = original;
    bool done = false;
    bool accepted = false;

    memset(&d, 0, sizeof d);
    d.kind = 1;
    d.title = (dos_char *)title;
    d.sub = 0;
    d.text = text;
    d.cx = d.cy = d.w = d.lines = -1;

    outer = keyboard_chain_active();
    keyboard_chain_enable();
    ui_overlay_show();
    sound_start();
    menu_state = menu_list_active();
    menu_list_disable();
    keyboard_buffer_drain();
    volume_text(text, sizeof text, name, value);
    dialog_draw(&d, 1);

    while (!done) {
        key = keyboard_read_blocking_hotkeys();
        switch (key) {
        case 0x14b:
            value -= 10;
            if (value < 0) value = 0;
            if (music) port_settings_preview_music_volume(value);
            else port_settings_preview_sound_volume(value);
            break;
        case 0x14d:
            value += 10;
            if (value > 200) value = 200;
            if (music) port_settings_preview_music_volume(value);
            else port_settings_preview_sound_volume(value);
            break;
        case 13:
            accepted = true;
            done = true;
            break;
        case 27:
            done = true;
            break;
        default:
            break;
        }
        if (!done && (key == 0x14b || key == 0x14d)) {
            dialog_restore_screen();
            volume_text(text, sizeof text, name, value);
            dialog_draw(&d, 0);
        }
    }

    if (accepted) {
        if (music) port_settings_commit_music_volume(value);
        else port_settings_commit_sound_volume(value);
    } else if (music) {
        port_settings_preview_music_volume(original);
    } else {
        port_settings_preview_sound_volume(original);
    }
    dialog_restore_screen();
    if (menu_state) menu_list_enable();
    sound_request_count_dec();
    ui_overlay_hide();
    if (!outer) keyboard_chain_disable();
    keyboard_buffer_drain();
    return 1; /* redraw the Options panel, keep F3 open */
}

dos_int port_options_music_volume(void)
{
    return volume_dialog("Music Volume", "Music volume", port_settings_music_volume(), true);
}

dos_int port_options_sound_volume(void)
{
    return volume_dialog("Sound Effects Volume", "Sound effects volume",
                         port_settings_sound_volume(), false);
}

dos_int port_options_interpolation(void)
{
    struct dialog d = {
        .kind = 7,
        .title = (dos_char *)"Interpolation",
        .sub = 1,
        .text = (dos_char *)"Do you want interpolation on?",
        .initial = (dos_uchar)(port_settings_interpolation() ? 1 : 0),
        .cx = -1, .cy = -1, .w = -1, .lines = -1
    };
    dos_int choice = dialog_run(&d);
    if (choice >= 0)
        port_settings_commit_interpolation(choice != 0);
    return 1;
}
