/* slots.c -- portable port of src/SLOTS.C: save-slot table load, save,
 * find and delete.
 * One translation unit; the sections below were the separate member
 * sources of grouped module C_A09D_A24E and keep their original ids.
 */
#include "game.h"

/* ---- F_A09D (original code at 0xA09D) ---- */
dos_int slot_menu_draw_header(void)
{
    dos_int i;

    resource_load_record_into(61, (uint8_t *)slot_table);
    resource_load_record_into(62, (uint8_t *)slot_transfer_table);
    resource_load_record(60);
    for (i = 0; i < 3; i++)
        gfx_blit_bitmap(i * 18, 400, (const uint8_t *)(ui_gfx_shadow_a + ((dos_int *)ui_gfx_shadow_a)[i] + 2));
    resource_load_record(59);
    gfx_blit_bitmap(0, 440, ui_gfx_shadow_a);
    resource_load_record(58);
    gfx_blit_bitmap(0, 457, ui_gfx_shadow_a);
    resource_load_record(57);
    return 0;   /* PORT: value unused (K&R implicit int) */
}

/* ---- F_A13F (original code at 0xA13F) ----
 * PORT: src/RESOURCE.C's resource_file_write_record(index, data) patches
 * record `index` of archive AE00x (top nibble of `index` selects the
 * directory) in place -- it lseeks to the record's own offset in the .DAT
 * file (read from the archive's directory table) and writes length-2
 * bytes there.  That is how save slots persist historically: the slot
 * table is written back INTO the archive.  Implementing that
 * update-in-place primitive is out of this file's scope (portable/
 * resource owns the archive; resource.h's read-only resource_load_record*
 * API has no write side).  Per the supervisor, this calls a dosio-based
 * helper `slot_file_write_record(index, data)` (declared extern below,
 * supplied by the supervisor) that will write a per-record overlay file
 * ("AE00x_<index>.sav" in a writable save directory), with the loader
 * preferring the overlay when present.
 *
 * Where the game reads this back: src/SLOTS.C's own slot_menu_draw_header
 * (F_A09D, above in this file) calls `resource_load_record_into(61,
 * slot_table)` and `resource_load_record_into(62, slot_transfer_table)`
 * -- record indices 0x3D/0x3E, the SAME two indices this function writes.
 * So resource_load_record()/resource_load_record_into() is the read path
 * for the same overlay this write path needs to produce. */

void slot_table_save(void)
{
    resource_file_write_record(0x3d, slot_table);   /* PORT: overlay-file persistence, resource.h */
    resource_file_write_record(0x3e, slot_transfer_table);
}

/* ---- F_A15E (original code at 0xA15E) ---- */
void slot_row_highlight(dos_int n)
{
    gfx_fill_rect(0x28, n * 11 + 0x3c, 0xf5, 10);
    gfx_box(0x28, n * 11 + 0x3c, 0xf5, 10);
}

/* ---- F_A19D (original code at 0xA19D) ---- */
void player_type_toggle_draw(void)
{
    gfx_fill_rect(40, 126, 238, 10);
    gfx_fill_rect(40, 136, 238, 10);
    gfx_box(40, 126, 238, 20);
}

/* ---- F_A1E0 (original code at 0xA1E0) ---- */
void quit_confirm_toggle_draw(void)
{
    gfx_fill_rect(50, 103, 218, 10);
    gfx_fill_rect(50, 113, 218, 10);
    gfx_box(50, 103, 218, 20);
}

/* ---- F_A223 (original code at 0xA223) ---- */
dos_int slot_find_free(void)
{
    dos_int i;

    for (i = 0; i < 10; i++)
        if (slot_table[i].text[0] == 0)
            return i;
    return 10;
}

/* ---- F_A24E (original code at 0xA24E) ---- */
void slot_delete(dos_int at)
{
    dos_int i;
    for (i = at + 1; i < 10; i++)
        slot_table[i - 1] = slot_table[i];
    gc563 = 0;
}
