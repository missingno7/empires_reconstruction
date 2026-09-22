/* menuloop.c -- portable port of src/MENULOOP.C: the menu loop and
 * shared-frame cleanup.  Exact reconstruction; local declaration and
 * comparison evaluation order preserved.
 */
#include "game.h"

#define g0fecat ((struct gc0fe_catalog *)gc0fe)

/* PORT: local typedef (tu-porting-rules.md sec 2, function-pointer
 * tables).  game_structs.h's `struct gc0fe_record` declares `callbacks` as
 * a single `void (*callbacks)(void)`, but this function indexes it as
 * `p->callbacks[row]` -- a per-row handler ARRAY, matching F_7964's
 * historical usage documented in that header's own comment.  Reinterpreted
 * via the pragma-guarded cast below (function-pointer -> object-pointer,
 * MSVC C4054); reported to the supervisor as a game_structs.h field-type
 * gap (the field should be `dos_int (**callbacks)(void)` or similar). */
typedef dos_int (*menu_row_callback_fn)(void);

void menu_loop_run(dos_int initial)
{
    dos_int outer, state, oldselected;
    struct gc0fe_record *p;
    dos_int oldrow, done, key, menu_result, cleared;
    dos_int selected, row;
    menu_row_callback_fn *cbs;

    selected = initial;
    if (selected < 0 || selected >= g0fecat->count)
        return;
    state = menu_list_active();
    menu_list_disable();
    outer = keyboard_chain_active();
    keyboard_chain_enable();
    keyboard_buffer_drain();
    ui_overlay_show();
    sound_start();
    while (selected != -1) {
        oldselected = selected;
        p = &g0fecat->records[selected];
        menu_list_draw(selected);
        dialog_draw_panel(p);
        cleared = 0;
        row = 0;
        done = 0;
        while (!done) {
            oldrow = row;
            gfx_fill_rect(dialog_box_x + 4, row * 10 + 17, dialog_box_w - 10, 10);
            gfx_box(dialog_box_x + 4, row * 10 + 17, dialog_box_w - 10, 10);
            key = keyboard_read_blocking_hotkeys();
            switch (key) {
            case 0x148: if (--row < 0) row = p->count - 1; break;
            case 0x150: if (++row >= p->count) row = 0; break;
            case 0x14b: if (--selected < 0) selected = g0fecat->count - 1; done = 1; break;
            case 0x14d: if (g0fecat->count <= ++selected) selected = 0; done = 1; break;
            case 0x13b: case 0x13c: case 0x13d: case 0x13e: case 0x13f:
            case 0x140: case 0x141: case 0x142: case 0x143: case 0x144:
                selected = key - 0x13b;
                if (selected < 0 || selected >= g0fecat->count) selected = oldselected;
                else if (selected != oldselected) done = 1;
                break;
            case 27: done = 1; selected = -1; break;
            case 13:
                dialog_restore_screen(); cleared = 1; menu_list_draw(-1);
#pragma warning(push)
#pragma warning(disable: 4054) /* function pointer -> data pointer: see file comment */
                cbs = (menu_row_callback_fn *)p->callbacks;
#pragma warning(pop)
                if (cbs[row]) menu_result = cbs[row](); else menu_result = 1;
                switch (menu_result) {
                case 0: default: done = 1; selected = -1; break;
                case 1: done = 1; break;
                }
                break;
            }
            if (!done) {
                gfx_fill_rect(dialog_box_x + 4, oldrow * 10 + 17, dialog_box_w - 10, 10);
                gfx_box(dialog_box_x + 4, oldrow * 10 + 17, dialog_box_w - 10, 10);
            }
        }
        if (!cleared) dialog_restore_screen();
    }
    menu_list_draw(-1);
    if (state) menu_list_enable();
    sound_request_count_dec();
    ui_overlay_hide();
    if (!outer) keyboard_chain_disable();
    keyboard_buffer_drain();
}
