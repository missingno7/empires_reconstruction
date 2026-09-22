/* Structural test for the portable generated-menu overlay. */
#include "game_structs.h"
#include "game_data.h"
#include "port_options.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void check(int condition, const char *what)
{
    if (!condition) {
        fprintf(stderr, "test_port_options: FAIL: %s\n", what);
        failures++;
    }
}

int main(void)
{
    void *top_level_text = menu_records_0CFA[0].text;
    void *top_level_callbacks = menu_records_0CFA[0].callbacks;
    port_options_install();
    check(menu_records_0CFA[2].count == PORT_OPTIONS_ROW_COUNT &&
          menu_records_0D3C[2].count == PORT_OPTIONS_ROW_COUNT,
          "both Options records have six rows");
    check(menu_records_0CFA[2].text == port_options_menu_text &&
          menu_records_0D3C[2].text == port_options_menu_text,
          "both Options records use port-owned text");
    check(menu_records_0CFA[2].callbacks == port_options_menu_callbacks &&
          menu_records_0D3C[2].callbacks == port_options_menu_callbacks,
          "both Options records use port-owned callbacks");
    check(strcmp((char *)port_options_menu_text,
                 "New User Messages\nMusic\nMusic Volume\nSound Effects\n"
                 "Sound Effects Volume\nInterpolation") == 0,
          "overlay text has the requested rows");
    check(menu_records_0CFA[0].text == top_level_text &&
          menu_records_0CFA[0].callbacks == top_level_callbacks,
          "other top-level menu records remain unchanged");
    if (failures) return 1;
    puts("test_port_options: OK");
    return 0;
}
