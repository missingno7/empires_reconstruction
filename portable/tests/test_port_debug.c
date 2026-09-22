/* Focused tests for the opt-in portable F4 Debug overlay and runtime state. */
#include "game_structs.h"
#include "game_data.h"
#include "game_funcs.h"
#include "gfx.h"
#include "port_debug.h"
#include "port_options.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void check(int condition, const char *what)
{
    if (!condition) {
        fprintf(stderr, "test_port_debug: FAIL: %s\n", what);
        failures++;
    }
}

static int record_equal(const struct menu_record *a, const struct menu_record *b)
{
    return a->label == b->label && a->label_width == b->label_width &&
           a->count == b->count && a->text == b->text &&
           a->callbacks == b->callbacks && a->width == b->width && a->x == b->x;
}

static void test_disabled_then_overlay(void)
{
    struct menu_record before_a[3];
    struct menu_record before_b[3];
    struct menu_record *generated_a = menu_records_0CFA;
    struct menu_record *generated_b = menu_records_0D3C;

    port_debug_init(false);
    port_debug_install_menu();
    check(!port_debug_enabled(), "debug is disabled");
    check(g0d36.count == 3 && g0d78.count == 3, "disabled debug keeps three top-level records");
    check(g0d36.records == generated_a && g0d78.records == generated_b,
          "disabled debug keeps generated catalog pointers");

    port_options_install();
    memcpy(before_a, g0d36.records, sizeof before_a);
    memcpy(before_b, g0d78.records, sizeof before_b);
    port_debug_init(true);
    port_debug_install_menu();
    check(port_debug_enabled(), "debug is enabled");
    check(g0d36.count == 4 && g0d78.count == 4, "enabled debug installs four top-level records");
    check(g0d36.records != generated_a && g0d78.records != generated_b,
          "enabled debug redirects both catalogs to port-owned arrays");
    for (int i = 0; i < 3; i++) {
        char what[80];
        snprintf(what, sizeof what, "catalog A record %d preserved", i);
        check(record_equal(&g0d36.records[i], &before_a[i]), what);
        snprintf(what, sizeof what, "catalog B record %d preserved", i);
        check(record_equal(&g0d78.records[i], &before_b[i]), what);
    }
    check(g0d36.records[2].count == PORT_OPTIONS_ROW_COUNT &&
          g0d36.records[2].text == port_options_menu_text &&
          g0d36.records[2].callbacks == port_options_menu_callbacks,
          "catalog A keeps the patched six-row F3 Options record");
    check(g0d78.records[2].count == PORT_OPTIONS_ROW_COUNT &&
          g0d78.records[2].text == port_options_menu_text &&
          g0d78.records[2].callbacks == port_options_menu_callbacks,
          "catalog B keeps the patched six-row F3 Options record");

    check(g0d36.records[3].count == 3 && g0d78.records[3].count == 3,
          "Debug submenu has exactly three rows");
    check(strcmp((char *)g0d36.records[3].text,
                 "Complete Chamber\nCollect All Pieces\nUnlimited Energy") == 0,
          "Debug submenu text is exact");
    check(g0d36.records[3].callbacks == port_debug_menu_callbacks &&
          g0d78.records[3].callbacks == port_debug_menu_callbacks,
          "both Debug records use the two portable callbacks");
    check(g0d36.records[3].x == 220 && g0d36.records[3].label_width == 78,
          "Debug tab occupies the right-hand top-bar region");
    check(g0d36.records[3].x + g0d36.records[3].label_width + 2 <= 320,
          "Debug tab fits in the 320px top bar");
    check(320 - (g0d36.records[3].width + 10) == 150,
          "Debug submenu has a calculable on-screen clamp position");

    /* Reinitialization to disabled restores the original catalog pointers. */
    port_debug_init(false);
    check(g0d36.count == 3 && g0d78.count == 3,
          "disabling after installation restores three records");
    check(g0d36.records == generated_a && g0d78.records == generated_b,
          "disabling after installation restores generated catalogs");
}

static void test_runtime_state(void)
{
    port_debug_init(false);
    check(!port_debug_unlimited_energy(), "Unlimited Energy starts off when disabled");
    check(!port_debug_take_complete_chamber_request(), "completion request starts empty");
    port_debug_set_unlimited_energy(true);
    port_debug_request_complete_chamber();
    check(!port_debug_unlimited_energy() &&
          !port_debug_take_complete_chamber_request(),
          "disabled debug cannot enable cheats or queue completion");
    port_debug_request_collect_all_pieces();
    check(!port_debug_take_collect_all_pieces_request(),
          "disabled debug cannot queue piece collection");
    check(port_debug_energy_delta(-1) == -1, "disabled debug preserves negative energy delta");

    port_debug_init(true);
    check(!port_debug_unlimited_energy(), "Unlimited Energy resets off at process initialization");
    check(port_debug_energy_delta(-1) == -1, "Unlimited Energy off preserves -1");
    port_debug_set_unlimited_energy(true);
    check(port_debug_energy_delta(-1) == 0 && port_debug_energy_delta(-2) == 0,
          "Unlimited Energy filters negative deltas");
    check(port_debug_energy_delta(0) == 0 && port_debug_energy_delta(1) == 1,
          "Unlimited Energy preserves queries and positive deltas");
    port_debug_request_complete_chamber();
    check(port_debug_take_complete_chamber_request(), "completion request is consumed once");
    check(!port_debug_take_complete_chamber_request(), "completion request does not leak");
    port_debug_request_collect_all_pieces();
    check(port_debug_take_collect_all_pieces_request(), "piece collection request is consumed once");
    check(!port_debug_take_collect_all_pieces_request(), "piece collection request does not leak");
    port_debug_set_unlimited_energy(false);
    check(port_debug_energy_delta(-1) == -1, "turning Unlimited Energy off restores deductions");
}

static void test_panel_geometry(void)
{
    static uint8_t fake_font[512];
    struct menu_record debug = {
        .width = 160,
        .x = 220
    };
    struct dialog q;
    dos_int panel_x;

    /* dialog_layout only needs the font width table for this kind-2 panel;
     * one-pixel widths make the test independent of the loaded asset font. */
    memset(fake_font, 1, sizeof fake_font);
    gc0e0 = fake_font;
    gc0e4 = 0;
    q.kind = 2;
    q.title = 0;
    q.sub = 1;
    q.text = port_debug_menu_text;
    q.initial = 0;
    q.cx = debug.x;
    q.cy = 13;
    q.w = debug.width;
    q.lines = 3;
    dialog_layout(&q);
    panel_x = q.cx;
    if (panel_x + dialog_box_w > 320)
        panel_x = 320 - dialog_box_w;
    if (panel_x < 0)
        panel_x = 0;
    check(panel_x + dialog_box_w <= 320,
          "Debug submenu outer frame fits inside the logical screen");
    check(panel_x == 150, "Debug submenu shifts left without moving its top tab");
}

int main(void)
{
    test_disabled_then_overlay();
    test_runtime_state();
    test_panel_geometry();
    if (failures) {
        fprintf(stderr, "test_port_debug: %d failure(s)\n", failures);
        return 1;
    }
    puts("test_port_debug: OK");
    return 0;
}
