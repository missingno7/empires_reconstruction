/* src/HITTEST.C: Tile and sprite hit testing. */
#include "game.h"

struct B3 { dos_uchar a, b, c; };

/* ---- F_5A3B (original code at 0x5A3B) ---- */
/* F_5A3B -- arm the 0x18-slot cursor/marker trail at the current hot spot,
   but only when that spot is inside the play window.  The four-term guard
   is an OR of the four out-of-window tests with a plain `return`, so the
   last term is inverted and jumps INTO the body while the fall-through is
   the return's jump to the epilogue. */
void cursor_trail_arm()
{
    dos_int i, x, y;

    gc04e = 0x17;
    x = cursor_x + 0x10;
    y = cursor_y + 4;
    if (x < 8 || x > 0x137 || y < 0x10 || y > 0x9f)
        return;
    for (i = 0; i < 0x18; i++) {
        tx[i] = x;
        ty[i] = y;
    }
    if (cursor_facing_left)
        raycast_beam_direction_index = 9;
    else
        raycast_beam_direction_index = 3;
    gc0b0 = 0;
    gc0c0 = 0x18;
    raycast_trail_active = 1;
    gc0b6 = 0;
}


/* ---- F_5AC3 (original code at 0x5AC3) ---- */
/* F_5AC3 -- the beam walk: step the ray eight times through the tile grid,
   test each step against the shadow bitmap and the object list, then rebuild
   the dirty rectangle from the 24 recorded points. */
void board_raycast_step()
{
    dos_int savemode;                       /* bp-1A */
    dos_int i;                              /* bp-18 */
    dos_int fx;                             /* bp-16 */
    dos_int fy;                             /* bp-14 */
    dos_int dx;                             /* bp-12 */
    dos_int dy;                             /* bp-10 */
    dos_int cell;                           /* bp-0E */
    dos_int recheck;                        /* bp-0C */
    dos_int obj;                            /* bp-0A */
    dos_int hit;                            /* bp-08 */
    dos_int live;                           /* bp-06 */
    struct B3 b;                            /* bp-04 .. bp-02 */
    dos_int x, y;                  /* si, di */

    recheck = 1;
    live = 0;
    savemode = cur_color_index_get();
    gc0be = 0;
    x = tx[gc04e];
    y = ty[gc04e];
    fx = x & 7;
    fy = y & 7;
    cell = ((y >> 3) - 2) * 0x26 + (x >> 3) - 1;
    for (i = 0; i < 8; i++) {
        if (x != 0) {
            /* PORT: `((int far *)((char far *)wig + raycast_beam_direction_index
               * 0x18))[gc0b0]` -- wig is `int wig[24][12]` historically
               (include/GC316... actually include/G0DCC.H is unrelated; see
               docs/portable/state-map.md "Porting notes": the true DATA
               image only backs 12 rows, so the generator clips the outer
               dimension to 12 (`dos_int wig[12][12]`, portable/generated/
               game_data.h), stride unchanged at 24 bytes/12 dos_int per row.
               A row is 12 dos_int = 24 bytes, so
               `(char far *)wig + idx*0x18` is exactly `&wig[idx][0]`, and
               indexing that as `(int far *)...[k]` is `wig[idx][k]` --
               confirmed equivalent.  raycast_beam_direction_index is kept in
               0..11 by the wrap loop below, matching the clipped 12 rows. */
            dx = wig[raycast_beam_direction_index][gc0b0];
            gc0b0++;
            dy = wig[raycast_beam_direction_index][gc0b0];
            gc0b0++;
            if (gc0b0 >= 12) gc0b0 = 0;
            x += dx;
            y += dy;
            fx += dx;
            fy += dy;
            if (fx & 8) {
                if (fx < 0) cell--;
                else cell++;
                fx &= 7;
                recheck = 1;
            }
            if (fy & 8) {
                if (fy < 0) cell -= 0x26;
                else cell += 0x26;
                fy &= 7;
                recheck = 1;
            }
            if (x < 8 || x > 0x137 || y < 0x10 || y > 0x9f) {
                x = 0;
            } else if (recheck != 0) {
                if (board_records[cell] & 7) x = 0;
                recheck = 0;
            }
            if ((i & 3) == 0 && x != 0) {
                if ((obj = rect_table_hit_id(x >> 1, y, 1, 1)) >= 8 && obj <= 0x1f) {
                    if ((gc0ba = record_field_skip_n(obj - 8))[1] == 2) {
                        gc0be = 1;
                        x = 0;
                        live = gc0b6 = 0;
                        goto after;
                    }
                }
                if (obj >= 0x30 && obj <= 0x4f) {
                    live = 1;
                    b = *(struct B3 *)(objtab + (obj - 0x30) * 3 + 1);
                    gc0b2 = b.a << 1;
                    gc0b4 = b.b;
                    gc0c2 = b.c & 0x1f;
                    gc0c4 = sprite_tile_bank + gc0c2 * 0x1e6 + 0x24;
                } else {
                    live = gc0b6 = 0;
                }
            }
after:
            if (live != 0 && gc0b6 == 0) {
                if ((hit = board_raycast_hit_test(x, y)) == 1) {
                    raycast_beam_direction_index = gc0c2 - raycast_beam_direction_index;
                    gc0b6 = 1;
                    stream_control_block_arm(15);
                } else if (hit == 2) {
                    raycast_beam_direction_index = gc0c2 - raycast_beam_direction_index - 8;
                    gc0b6 = 1;
                    stream_control_block_arm(15);
                } else if (hit == 3) {
                    raycast_beam_direction_index = gc0c2 - raycast_beam_direction_index + 8;
                    gc0b6 = 1;
                    stream_control_block_arm(15);
                }
                if (gc0b6 != 0) {
                    while (raycast_beam_direction_index < 0) raycast_beam_direction_index += 12;
                    while (raycast_beam_direction_index >= 12) raycast_beam_direction_index -= 12;
                }
            }
        } else {
            x = 0;
            if (--gc0c0 <= 0) raycast_trail_active = 0;
        }
        if (++gc04e >= 0x18) gc04e = 0;
        tx[gc04e] = x;
        ty[gc04e] = y;
    }
    gc046 = 0x140;
    gc04a = 0xc8;
    gc048 = 0;
    gc04c = 0;
    gfx_color_select(14);
    for (i = 0; i < 0x18; i++) {
        if ((x = tx[i]) != 0) {
            gfx_set_pixel(x, y = ty[i]);
            if (x < gc046) gc046 = x;
            if (gc048 < x) gc048 = x;
            if (y < gc04a) gc04a = y;
            if (gc04c < y) gc04c = y;
        }
    }
    if (gc048 != 0) {
        /* PORT: `edge` (src/HITTEST.C local extern, `char far *edge;
           /@ DS:40C4 @/`) and `rect_queue_write_ptr` (used explicitly in
           sprite_slots_redraw below) are the SAME DS:40C4 object under two
           historical local names -- rect_queue_write_ptr is subsystem-owned
           by portable/gfx (docs/portable/architecture.md, portable/include/
           gfx.h), so this uses that one canonical name instead of
           redeclaring `edge`. */
        *rect_queue_write_ptr = (dos_uchar)(gc046 >> 1);
        rect_queue_write_ptr++;
        *rect_queue_write_ptr = (dos_uchar)gc04a;
        rect_queue_write_ptr++;
        *rect_queue_write_ptr = (dos_uchar)((gc048 - gc046 + 4) >> 1);
        rect_queue_write_ptr++;
        *rect_queue_write_ptr = (dos_uchar)((dos_char)gc04c - (dos_char)gc04a + 1);
        rect_queue_write_ptr++;
    }
    gfx_color_select(savemode);
}


/* ---- F_5E98 (original code at 0x5E98) ---- */
/* F_5E98 -- redraw the 0x18 sprite slots, then append four bytes to the
   command stream at rect_queue_write_ptr.  Plain C. */
void sprite_slots_redraw(void)
{
    dos_int w4, saved;
    dos_int i, p;

    saved = cur_color_index_get();
    for (i = 0; i < 0x18; i++) {
        if ((p = tx[i]) != 0) {
            result = gfx_get_pixel(p, (w4 = ty[i]) + 0xb8);
            gfx_set_pixel(p, w4);
        }
    }
    if (gc048 != 0) {
        *rect_queue_write_ptr++ = (dos_uchar)(gc046 >> 1);
        *rect_queue_write_ptr++ = *(dos_uchar *)&gc04a;
        *rect_queue_write_ptr++ = (dos_uchar)((gc048 - gc046 + 4) >> 1);
        *rect_queue_write_ptr++ = (dos_uchar)(*(dos_char *)&gc04c - *(dos_char *)&gc04a + 1);
    }
    gfx_color_select(saved);
}


/* ---- F_5F3C (original code at 0x5F3C) ---- */
dos_int board_raycast_hit_test(dos_int x, dos_int y)
{
    dos_char c;
    if (display_mode == 5) {
        switch (gfx_get_pixel(x, y + 0xb8)) {
        case 0x83: return 1;
        case 0x86: return 2;
        case 0x8a: return 3;
        default: return 0;
        }
    } else {
        x -= gc0b2;
        y -= gc0b4;
        c = gc0c4[(y << 4) - y + (x >> 1)];
        if (x & 1) c &= 15;
        else c = (dos_char)((c >> 4) & 15);
        if (display_mode == 2) {
            switch (c) {
            case 2: return 1;
            case 3: return 2;
            case 4: return 3;
            default: return 0;
            }
        } else {
            switch (c) {
            case 11: return 1;
            case 9: return 2;
            case 8: return 3;
            default: return 0;
            }
        }
    }
}


/* ---- F_6021 (original code at 0x6021) ---- */
/* F_6021 -- frameless (no parameters, no locals). */
void board_unit_script_trigger(void)
{
    /* PORT: the historical call `board_run_unit_script((int)gc0ba, gc0bc);`
       passes TWO arguments through the unprototyped local extern
       `void board_run_unit_script();`, but the real definition
       (src/BOARD.C:653, `void board_run_unit_script(unsigned char far *s)`,
       matching game_funcs.h's single-parameter prototype) only ever reads
       ONE far-pointer parameter off the stack -- gc0bc was a dead second
       argument (harmless under cdecl, caller-cleans-stack).  Per
       tu-porting-rules.md sec "If game_funcs.h's prototype disagrees with
       how your file calls a function, follow the DEFINITION": drop the
       (int) cast and the dead gc0bc argument, pass gc0ba (now a real
       dos_char*) directly. */
    if (gc0be)
        board_run_unit_script((dos_uchar *)gc0ba);
}
