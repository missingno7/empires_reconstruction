/* port_debug.c -- opt-in portable F4 Debug menu and runtime state. */
#include "game.h"

#include "port_debug.h"

#include <stdbool.h>
#include <string.h>

static bool s_enabled;
static bool s_unlimited_energy;
static bool s_complete_chamber;
static bool s_menu_installed;
static struct menu_record *s_saved_records_a;
static struct menu_record *s_saved_records_b;
static dos_int s_saved_count_a;
static dos_int s_saved_count_b;

dos_char port_debug_menu_text[] =
    "Complete Chamber\nUnlimited Energy\0";

static dos_char port_debug_label[] = "Debug F4\0";

dos_int port_debug_complete_chamber(void)
{
    port_debug_request_complete_chamber();
    return 0; /* close the F-key menu so gameplay can consume the request */
}

dos_int port_debug_unlimited_energy_dialog(void)
{
    struct dialog d = {
        .kind = 7,
        .title = (dos_char *)"Unlimited Energy",
        .sub = 1,
        .text = (dos_char *)"Do you want unlimited energy?",
        .initial = (dos_uchar)(port_debug_unlimited_energy() ? 1 : 0),
        .cx = -1, .cy = -1, .w = -1, .lines = -1
    };
    dos_int choice = dialog_run(&d);

    if (choice >= 0)
        port_debug_set_unlimited_energy(choice != 0);
    return 1; /* redraw the Debug submenu */
}

void (*port_debug_menu_callbacks[2])(void) = {
    (void (*)(void))port_debug_complete_chamber,
    (void (*)(void))port_debug_unlimited_energy_dialog
};

void port_debug_init(bool enabled)
{
    if (s_menu_installed && !enabled) {
        g0d36.records = s_saved_records_a;
        g0d36.count = s_saved_count_a;
        g0d78.records = s_saved_records_b;
        g0d78.count = s_saved_count_b;
        s_menu_installed = false;
    }
    s_enabled = enabled;
    s_unlimited_energy = false;
    s_complete_chamber = false;
}

bool port_debug_enabled(void)
{
    return s_enabled;
}

bool port_debug_unlimited_energy(void)
{
    return s_unlimited_energy;
}

void port_debug_set_unlimited_energy(bool enabled)
{
    if (s_enabled)
        s_unlimited_energy = enabled;
}

dos_int port_debug_energy_delta(dos_int delta)
{
    if (s_enabled && s_unlimited_energy && delta < 0)
        return 0;
    return delta;
}

void port_debug_request_complete_chamber(void)
{
    if (s_enabled)
        s_complete_chamber = true;
}

bool port_debug_take_complete_chamber_request(void)
{
    bool requested = s_complete_chamber;
    s_complete_chamber = false;
    return requested;
}

void port_debug_install_menu(void)
{
    static struct menu_record menu_a[4];
    static struct menu_record menu_b[4];
    struct menu_record *source_a;
    struct menu_record *source_b;

    if (!s_enabled || s_menu_installed)
        return;

    source_a = g0d36.records;
    source_b = g0d78.records;
    s_saved_records_a = source_a;
    s_saved_records_b = source_b;
    s_saved_count_a = g0d36.count;
    s_saved_count_b = g0d78.count;
    memcpy(menu_a, source_a, sizeof menu_a[0] * 3u);
    memcpy(menu_b, source_b, sizeof menu_b[0] * 3u);

    menu_a[3] = (struct menu_record){
        .label = port_debug_label,
        .label_width = 78,
        .count = 2,
        .text = port_debug_menu_text,
        .callbacks = port_debug_menu_callbacks,
        .width = 140,
        .x = 220
    };
    menu_b[3] = menu_a[3];

    g0d36.records = menu_a;
    g0d36.count = 4;
    g0d78.records = menu_b;
    g0d78.count = 4;
    s_menu_installed = true;
}
