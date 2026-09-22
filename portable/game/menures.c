/* menures.c -- portable port of src/MENURES.C. */
#include "game.h"
#include "trace.h"

dos_int menu_resources_load(void)
{
    dos_int i, j;

    EMPIRES_TRACE("menu_resources_load g720=%d", g720);
    if (g720 == 1) return 0;   /* PORT: original had a bare `return;` on a
                                 * K&R implicit-int function; no caller was
                                 * found to depend on the return value. */
    else if (g720 == 2) level_free_descriptor_table();
    resource_load_record_alloc(20, (uint8_t **)&g7352);
    for (j = i = 0; i < 23; i++) resource_ptr_table[j++] = g7352 + ((dos_int *)g7352)[i] + 2;
    resource_load_record_alloc(21, (uint8_t **)&g7356);
    for (i = 0; i < 20; i++) resource_ptr_table[j++] = g7356 + ((dos_int *)g7356)[i] + 2;
    resource_load_record_alloc(22, (uint8_t **)&g735a);
    for (i = 0; i < 41; i++) resource_ptr_table[j++] = g735a + ((dos_int *)g735a)[i] + 2;
    resource_load_record_alloc(19, (uint8_t **)&sprite_tile_bank);
    resource_load_record(23);
    memmove(tile_width_table, ui_gfx_shadow_a, 84);
    resource_load_record(24);
    memmove(tile_height_table, ui_gfx_shadow_a, 84);
    resource_load_record(25);
    memmove(actor_sprite_dims_table, ui_gfx_shadow_a, 672);
    g720 = 1;
    EMPIRES_TRACE("menu_resources_load done: [0]=%p [43]=%p [63]=%p [83]=%p", (void *)resource_ptr_table[0], (void *)resource_ptr_table[43], (void *)resource_ptr_table[63], (void *)resource_ptr_table[83]);
    return 0;
}
