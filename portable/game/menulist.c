/* menulist.c -- portable port of src/MENULIST.C.
 *
 * The historical file locally declares `struct catalog_entry`/`struct S`
 * as its own (incomplete) view of the record gc0fe points at; these are
 * the exact same DS:0FE catalog game_structs.h already curates as
 * `struct gc0fe_catalog`/`struct gc0fe_record` (see that header's own
 * comment on gc0fe and portable/game/menuloop.c's `g0fecat` macro), so per
 * tu-porting-rules.md ("Struct tags come from game_structs.h only; never
 * re-declare a struct") this uses those types instead of re-declaring a
 * local shape.  catalog_entry.a/.b/.c correspond to gc0fe_record's
 * text/width/x fields.
 */
#include "game.h"

#define g0fecat ((struct gc0fe_catalog *)gc0fe)

void menu_list_draw(dos_int n)
{
    dos_int a, b, i, j;
    struct gc0fe_record r;
    dos_int w;
    dos_int x, y;

    a = cur_color_index_get();
    b = sprite_sheet_index_get();
    sprite_sheet_select(0);
    gfx_color_select(0);
    gfx_clear_rect(0, 0, 320, 13);
    gfx_color_select(15);
    gfx_clear_rect(1, 2, 318, 10);
    for (i = g0fecat->count - 1; i >= 0; i--) {
        r = g0fecat->records[i];
        w = r.width;
        x = r.x;
        y = 2;
        if (i == n) {
            gfx_color_select(12);
            gfx_clear_rect(x, y, w + 2, 10);
            gfx_color_select(15);
            text_draw_wrapped(++x, y, r.text);
        } else {
            for (j = 0; j < 2; j++) {
                gfx_color_select(7);
                gfx_vline(x, y, 9);
                gfx_bar(x, y + 9, w + 2);
                gfx_color_select(0);
                gfx_bar(x++, y-- + 9, 1);
            }
            rect_border_draw(x++, y, w + 2, 10);
            gfx_color_select(15);
            gfx_clear_rect(x, y, w, 9);
            gfx_color_select(0);
            text_draw_wrapped(x, y, r.text);
        }
    }
    gfx_box(0, 0, 320, 12);
    gfx_color_select(a);
    sprite_sheet_select(b);
}
