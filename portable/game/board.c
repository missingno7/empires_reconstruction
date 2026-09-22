/* board.c -- src/BOARD.C: board drawing, painting and the unit script
 * interpreter.  One translation unit; the sections below were the separate
 * member sources of grouped module C_200F_3986 and keep their original ids.
 *
 * generator type issues (report):
 *  - g6e88/gb1cc/g9990/g7904/g6ac4/g6ca6/g9ae0/g99da/g9c50/g9b6e/g9a5c/
 *    g7400/g735e: game_state.h emits each as `dos_char X[N][1]` (2D, inner
 *    dim 1, matching the historical `char X[][1]`), so `X[0]` (decaying to
 *    dos_char*) is the address of the one-and-only "row".  A few call
 *    sites in the historical source use the bare array name `X` instead of
 *    `X[0]` for the exact same address (e.g. board_update_moving_records'
 *    `gfx_copy_rect(x,y,g6ca6,0)`) -- that only type-checked historically
 *    because VIDEO.H declares gfx_copy_rect etc. completely unprototyped
 *    (`extern void gfx_copy_rect();`), so no argument conversion was
 *    enforced and a decayed `char(*)[1]` and a decayed `char*` are the
 *    same runtime address either way.  Every such bare-name site below is
 *    written `X[0]` to satisfy the portable (prototyped) gfx.h signatures.
 *  - xa/ya (aka w8bea/w8bf4, DS:8BEA/DS:8BF4): RESOLVED -- originally
 *    generated as dos_int[1] each while board_scan_wipe_effect below
 *    indexes xa[i]/ya[i] with i = the caller's board-object index
 *    (src/GAME.C passes `obj`, not always 0); flagged in an earlier report
 *    and now regenerated as dos_int[5] with g8bec/g8bee/g8bf6/g8bf8 as
 *    element-alias macros (xa[1]/xa[2]/ya[1]/ya[2]).  xa[i]/ya[i] below are
 *    unchanged (already the correct spelling; they were only out-of-bounds
 *    against the old, too-small declared size).
 *
 * `blit`/`wipe`/`copy`/`box` (local BOARD.C aliases, tagged with their
 * F_xxxx addresses in the historical externs) are the same functions as
 * gfx_blit_bitmap/gfx_wipe_rect/gfx_copy_rect/gfx_box (include/VIDEO.H's
 * own header comment gives the same F_03C9/F_03B4/F_03CC/F_039F mapping);
 * every call below uses the canonical gfx.h name directly.
 *
 * `src`/`dst` (DS:C5CA / DS:40C4) are the same objects as ui_gfx_blob and
 * rect_queue_write_ptr (resource.h / gfx.h) -- board_scroll_transition
 * later in this same historical file already spells the identical
 * operation `rect_queue_write_ptr = ui_gfx_blob;`, confirming the mapping;
 * every `dst = src;` below is written that way.
 */
#include "game.h"
#include "gfx_tween.h"

static void board_platform_copy_visible(int index, dos_int x, dos_int y, const uint8_t *bitmap)
{
    /* The board renderer first paints the platform into the off-screen
     * 488-row board and then wipes that region into the visible 200-row
     * viewport.  Only this final screen-space redraw is a tween object. */
    gfx_tween_tag = GFX_TWEEN_TAG_PLATFORM(index);
    gfx_copy_rect(x, y, bitmap, 0);
    gfx_tween_tag = 0;
}

/* ---- F_200F (original code at 0x200F) ---- */
void resource_icon_table_load(void)
{
    dos_int i;
    for (i = 0; i < 4; ++i)
        resource_load_record_into((dos_uint)(i + 5), (uint8_t *)a6f2a[i]);
    for (i = 0; i < 7; ++i)
        resource_load_record_into((dos_uint)(i + 0xa), (uint8_t *)a893c[i]);
    resource_load_record_into(9, (uint8_t *)g6e88[0]);
    resource_load_record_into(0x11, (uint8_t *)gb1cc[0]);
    resource_load_record_into(0x12, (uint8_t *)g9990[0]);
    resource_load_record_into(0x2e, (uint8_t *)g7904[0]);
    resource_load_record_into(0x2f, (uint8_t *)g6ac4[0]);
    resource_load_record_into(0x30, (uint8_t *)g6ca6[0]);
    resource_load_record_into(0x27, (uint8_t *)g9ae0[0]);
    resource_load_record_into(0x28, (uint8_t *)g99da[0]);
    resource_load_record_into(0x29, (uint8_t *)g9c50[0]);
    resource_load_record_into(0x2a, (uint8_t *)g9b6e[0]);
    resource_load_record_into(0x2b, (uint8_t *)g9a5c[0]);
    resource_load_record_into(0x2c, (uint8_t *)g7400[0]);
    resource_load_record_into(0x2d, (uint8_t *)g735e[0]);
}


/* ---- F_2119 (original code at 0x2119) ---- */
void resource_scoreboard_unpack(void)
{
    dos_char *d;
    dos_char *s;
    dos_int i;
    d = g2380[0];
    s = (dos_char *)(ui_gfx_shadow_a + 2);
    resource_load_record(0x26);
    for (i = 0; i < 8; ++i) {
        memmove(d, s, 0x82);
        d += 0x82;
        s += 0x84;
        memmove(d, s, 0x62);
        d += 0x62;
        s += 0x64;
        memmove(d, s, 0x92);
        d += 0x92;
        s += 0x94;
    }
}


/* ---- F_21A9 (original code at 0x21A9) ---- */
/* F_21A9 -- load the two boot sprite sheets and publish their far addresses
   into the DS:C0D6 table F_6CA6 indexes.  An array NAME assigned to a far
   pointer stores `ds` into the segment half and the offset CONSTANT into the
   other; the constant index folds to a direct displacement (rule 2). */
void sprite_load_boot_sheets(void)
{
    resource_load_record_into(0, g9cf2);
    resource_load_record_into(1, ga6b6);
    gc0d6[0] = g9cf2;
    gc0d6[1] = ga6b6;
}


/* ---- F_21DB (original code at 0x21DB) ---- */
/* F_21DB -- read record 4 into the staging block, allocate 15,502 bytes and
   copy 23 stripes of 674 bytes out of the staging block (676-byte pitch, two
   header bytes skipped) into it, then one more stripe into the DS:96EE
   buffer.  No frame: no parameter and no local (rule 11); the loop counter is
   the one register variable. */
void resource_stripe_table_load(void)
{
    dos_int i;

    resource_load_record(4);
    resource_stripe_table = malloc(0x3c8e);
    for (i = 0; i < 23; i++)
        movmem(ui_gfx_shadow_a + i * 676 + 2, resource_stripe_table + i * 674, 674);
    movmem(ui_gfx_shadow_a + 0x3cbe, g96ee, 674);
}


/* ---- F_224C (original code at 0x224C) ---- */
/* F_224C -- allocate the 0x55F0-byte arena, keep the far pointer at DS:99D6
   and run the three initialisers over it.  `xor dx,dx` before the push makes
   the size one 32-bit argument, not two ints, and the result comes back in
   DX:AX -- farmalloc, CC.LIB at IP 0E9B4h (substrate/LIB_FMALLOC.json). */
void hud_arena_init(void)
{
    g99d6 = malloc((size_t)0x55f0UL);   /* PORT: farmalloc(0x55f0L) -> malloc (rule 4/6) */
    resource_icon_table_load();
    resource_scoreboard_unpack();
    hud_icons_load();
}


/* ---- F_2269 (original code at 0x2269) ---- */
/* F_2269 -- blit the cursor sprite, and the pending one first.  gc99d2 is a
   far pointer into the 0x2a2-byte sprite table. */
void sprite_draw_cursor(void)
{
    if (g072c) {
        gfx_copy_rect(g0736, g0738, g96ee, 0);
        g072c--;
    }
    gfx_copy_rect(g0736, g0738, (const uint8_t *)(resource_stripe_table + g072e * 0x2a2), g073a);
}


/* ---- F_22B1 (original code at 0x22B1) ---- */
/* F_22B1 -- clamp the view window to the board and blit it.  Plain C.
   si/di are the two register variables, [bp-4] and [bp-2] the two ints. */
void board_redraw_view(void)
{
    dos_int w4, w2;
    dos_int x, y;

    if (cursor_x < 8) {
        x = 8;
        w4 = 0x28;
    } else if (cursor_x + 0x27 > 0x137) {
        x = cursor_x;
        w4 = 0x138 - cursor_x;
    } else {
        x = cursor_x;
        w4 = 0x28;
    }
    if (cursor_y < 0x10) {
        y = 0x10;
        w2 = 0x28;
    } else if (cursor_y + 0x27 > 0x9f) {
        y = cursor_y;
        w2 = 0xa0 - cursor_y;
    } else {
        y = cursor_y;
        w2 = 0x28;
    }
    gfx_wipe_rect(x, y + 0xb8, w4, w2, x, y);
}


/* ---- F_233E (original code at 0x233E) ---- */
void board_scan_wipe_effect(dos_int i)
{
    dos_int k;

    sound_stop_reset();
    hud_scroll_cooldown_ticks = 0;
    stream_control_block_arm(0x0d);
    g96 = 0x190;
    board_redraw_view();
    gbc = 1;
    for (k = 1; k < 5; k++) {
        timer_deadline_arm(0x18);
        rect_queue_write_ptr = ui_gfx_blob;
        /* PORT: cel is generated as a flat uint8_t[3965] (historical
         * `struct CEL cel[]`, 0x319 bytes/record, no portable struct yet --
         * see docs/portable/state-map.md); &cel[k] -> cel + k*0x319. */
        gfx_blit_bitmap(xa[i], ya[i] + 0xb8, cel + (size_t)k * 0x319);
        gfx_wipe_rect(xa[i], ya[i] + 0xb8, 0x2e, 0x21, xa[i], ya[i]);
        gfx_copy_rect(cursor_x, cursor_y, (const uint8_t *)(pool + g72e * 0x2a2), cursor_facing_left);
        rect_queue_flush();
        timer_deadline_wait();
    }
    board_redraw_view();
    for (k = 12; k < 16; k++) {
        timer_deadline_arm(0x18);
        rect_queue_write_ptr = ui_gfx_blob;
        gfx_wipe_rect(xa[i], ya[i] + 0xb8, 0x2e, 0x28, xa[i], ya[i]);
        board_redraw_view();
        gfx_copy_rect(xa[i] + 4, ya[i], (const uint8_t *)(pool + k * 0x2a2), cursor_facing_left);
        rect_queue_flush();
        timer_deadline_wait();
    }
    gbc = 0;
    stream_control_block_arm(0x0d);
    for (k = 3; k >= 0; k--) {
        timer_deadline_arm(0x18);
        gfx_blit_bitmap(xa[i], ya[i], cel + (size_t)k * 0x319);
        gfx_box(xa[i], ya[i], 0x2e, 0x21);
        timer_deadline_wait();
    }
    g96 = 0x9f;
}


/* ---- F_250C (original code at 0x250C) ---- */
/* F_250C -- decrement board_records[i*3+0x2ac]'s low nibble (a countdown
 * field), flipping bit 0x20 when it hits 0; K&R implicit-int, never
 * returns a value on any path (every call site is a discarded statement
 * expression -- see src/BOARD.C:703) so `return 0;` below is a
 * compile-cleanliness addition, not a semantic one. */
dos_int f250c(dos_int i)
{
    dos_uchar a;
    dos_uchar *p;
    p = (dos_uchar *)(board_records + i * 3 + 0x2ac);
    if ((a = *p) != 0) {
        if ((a &= 15) != 0) {
            *p = (dos_uchar)((*p & 0xf0) | (6 - a));
            if (*p & 15)
                *p ^= 32;
        } else {
            *p = (dos_uchar)((*p & 0xf0) | 6);
        }
    }
    return 0;
}


/* ---- F_257D (original code at 0x257D) ---- */
dos_int point_in_hotspot_rect(dos_int x, dos_int y)
{
    if (x >= cursor_x && x <= cursor_x + 0x1f && y >= cursor_y && y <= cursor_y + 0x27)
        return 1;
    return 0;
}


/* ---- F_25B3 (original code at 0x25B3) ---- */
/* Update the ten moving board records and redraw the changed cells. */
void board_update_moving_records(void)
{
    dos_int i, index;
    dos_uchar flag;
    dos_uchar *p;
    dos_uint x, y;
    p = (dos_uchar *)(board_records + 684);
    for (i = 0; i < 10; i++, p += 3) {
        if ((flag = *p) & 15) {
            if (!(--*p & 15))
                *p ^= 32;
            /* PORT: the historical `x=(unsigned)(char far *)p[1];` roundabout
             * casts a byte value through a never-dereferenced far pointer
             * purely to reach Turbo C's zero-extending codegen path (see
             * F_28AC's near-identical comment: "this intermediate preserves
             * Turbo C's AX:DX extension ... It is never dereferenced. This
             * compiler-specific expression is not provenance proof.").  The
             * observable result is the same as a direct zero-extending cast
             * of the byte, which is what a portable dos_uint conversion
             * already does. */
            x = (dos_uint)p[1];
            x <<= 1;
            y = (dos_uint)p[2];
            index = (dos_int)((x >> 3) + ((y >> 3) - 2) * 38 - 1);
            y += 180;
            x -= 4;
            if (flag & 128) {
                if (flag & 32) {
                    if (point_in_hotspot_rect((dos_int)(x + 4), (dos_int)(y - 192)))
                        *p = flag;
                    else {
                        gfx_wipe_rect((dos_int)x, (dos_int)(y + 144), 16, 56, (dos_int)x, (dos_int)y);
                        y -= 8;
                        p[2] -= 8;
                        g96 = 359; gfx_copy_rect((dos_int)x, (dos_int)y, (const uint8_t *)g6ca6[0], 0); g96 = 159;
                        gfx_wipe_rect((dos_int)x, (dos_int)y, 16, 64, (dos_int)x, (dos_int)(y - 184));
                        board_platform_copy_visible(i, (dos_int)x, (dos_int)(y - 184), (const uint8_t *)g6ca6[0]);
                        board_records[index - 38] = 7;
                        board_records[index + 190] = 0;
                    }
                } else {
                    if (point_in_hotspot_rect((dos_int)(x + 4), (dos_int)(y - 120)))
                        *p = flag;
                    else {
                        gfx_wipe_rect((dos_int)x, (dos_int)(y + 144), 16, 56, (dos_int)x, (dos_int)y);
                        p[2] += 8;
                        g96 = 359; gfx_copy_rect((dos_int)x, (dos_int)(y + 8), (const uint8_t *)g6ca6[0], 0); g96 = 159;
                        gfx_wipe_rect((dos_int)x, (dos_int)y, 16, 64, (dos_int)x, (dos_int)(y - 184));
                        board_platform_copy_visible(i, (dos_int)x, (dos_int)(y - 184), (const uint8_t *)g6ca6[0]);
                        board_records[index + 228] = 7;
                        board_records[index] = 0;
                    }
                }
            } else {
                if (flag & 32) {
                    if (point_in_hotspot_rect((dos_int)(x - 8), (dos_int)(y - 180)))
                        *p = flag;
                    else {
                        gfx_wipe_rect((dos_int)x, (dos_int)(y + 144), 56, 16, (dos_int)x, (dos_int)y);
                        x -= 8;
                        p[1] -= 4;
                        g96 = 359; gfx_copy_rect((dos_int)x, (dos_int)y, (const uint8_t *)g6ac4[0], 0); g96 = 159;
                        gfx_wipe_rect((dos_int)x, (dos_int)y, 64, 16, (dos_int)x, (dos_int)(y - 184));
                        board_platform_copy_visible(i, (dos_int)x, (dos_int)(y - 184), (const uint8_t *)g6ac4[0]);
                        board_records[index - 1] = 7;
                        board_records[index + 5] = 0;
                    }
                } else {
                    if (point_in_hotspot_rect((dos_int)(x + 64), (dos_int)(y - 180)))
                        *p = flag;
                    else {
                        gfx_wipe_rect((dos_int)x, (dos_int)(y + 144), 56, 16, (dos_int)x, (dos_int)y);
                        p[1] += 4;
                        g96 = 359; gfx_copy_rect((dos_int)(x + 8), (dos_int)y, (const uint8_t *)g6ac4[0], 0); g96 = 159;
                        gfx_wipe_rect((dos_int)x, (dos_int)y, 64, 16, (dos_int)x, (dos_int)(y - 184));
                        board_platform_copy_visible(i, (dos_int)(x + 8), (dos_int)(y - 184), (const uint8_t *)g6ac4[0]);
                        board_records[index + 6] = 7;
                        board_records[index] = 0;
                    }
                }
            }
        }
    }
}


/* ---- F_28AC (original code at 0x28AC) ---- */
/* Exact Turbo C reconstruction of the ten-record board drawing loop.
 * The far-pointer intermediate retains the historical zero extension into
 * AX:DX before the low word is assigned to a coordinate. It is never
 * dereferenced. This compiler-specific expression is not provenance proof.
 * PORT: see board_update_moving_records above for why a direct zero-
 * extending dos_uint cast reproduces the same result. */
void board_mark_record_cells(void)
{
    dos_int i, index, stride, j;
    dos_uchar flag;
    dos_uchar *p;
    dos_uint x, y;
    p = (dos_uchar *)(board_records + 684);
    for (i = 0; i < 10; i++, p += 3) {
        if ((flag = *p) != 0) {
            x = (dos_uint)p[1];
            x <<= 1;
            y = (dos_uint)p[2];
            index = (dos_int)((x >> 3) + ((y >> 3) - 2) * 38 - 1);
            y += 180;
            x -= 4;
            if (flag & 128) {
                gfx_copy_rect((dos_int)x, (dos_int)y, (const uint8_t *)g6ca6[0], 0);
                stride = 38;
            } else {
                gfx_copy_rect((dos_int)x, (dos_int)y, (const uint8_t *)g6ac4[0], 0);
                stride = 1;
            }
            for (j = 0; j < 6; j++, index += stride)
                board_records[index] = 7;
        }
    }
}


/* ---- F_2986 (original code at 0x2986) ---- */
void f2986(dos_uchar *p)
{
    dos_int saved;
    dos_int n;
    dos_int col_off;   /* renamed from the historical `off` -- collides with the
                        * game_state.h macro `off` (alias for gca62, src/OPLVOICE.C) */
    dos_int di;
    dos_int i;

    saved = gbc;
    di = p[0] * 2;
    n = p[1];
    gfx_copy_rect(di, n + 0xb8, (const uint8_t *)gb1cc[0], 0);
    gbc = 0;
    i = p[4] + 5;
    while (i < 10) {
        if (p[i] == 0)
            break;
        col_off = (i - 5) * 10 + di + 6;
        gfx_copy_rect(col_off, n + 0xb9, (const uint8_t *)a893c[p[i] - 1], 0);
        ++i;
    }
    gbc = saved;
}


/* ---- F_2A2D (original code at 0x2A2D) ---- */
dos_char *record_field_skip_n(dos_int n)
{
    dos_uchar *p;
    dos_int i;

    p = (dos_uchar *)(record_table_root + (*record_table_root) * 4 + 2);
    for (i = 0; i < n; i++)
        p += *p;
    return (dos_char *)p;
}


/* ---- F_2A70 (original code at 0x2A70) ---- */
/* Walk the nested count-prefixed tables at record_table_root and return the near address of the fourth level. */
dos_uchar *record_table_level4_ptr(void)
{
    dos_uchar *p;
    p = (dos_uchar *)(record_table_root + *record_table_root * 4 + 1);
    p = (dos_uchar *)record_field_skip_n(*p);
    p += *p * 3 + 1;
    p += *p * 12 + 1;
    return p + *p * 3 + 1;
}


/* ---- F_2AE2 (original code at 0x2AE2) ---- */
/* F_2AE2 -- the board redraw.  Plain C.  The module is on the TASM path:
   the two  83 E3 3F  (and bx,3fh) at 2C31 and 2D78 are the short AND form
   TASM picks and TCC's own writer does not. */
void board_redraw_paint(void)
{
    dos_uchar c;
    dos_uchar *p;
    dos_uchar *q;
    dos_int w8;
    dos_int n;
    dos_int w4;
    dos_int w2;
    dos_int i, j;

    if (campaign_round_node_cursor == 0x2a)
        return;
    record_table_root = board_records + 0x2ca;
    gfx_blit_bitmap(8, 0xc8, (const uint8_t *)s8c12);
    gfx_blit_bitmap(0x54, 0xc8, (const uint8_t *)s8c12);
    gfx_blit_bitmap(0xa0, 0xc8, (const uint8_t *)s8c12);
    gfx_blit_bitmap(0xec, 0xc8, (const uint8_t *)s8c12);
    gfx_blit_bitmap(8, 0x110, (const uint8_t *)s8c12);
    gfx_blit_bitmap(0x54, 0x110, (const uint8_t *)s8c12);
    gfx_blit_bitmap(0xa0, 0x110, (const uint8_t *)s8c12);
    gfx_blit_bitmap(0xec, 0x110, (const uint8_t *)s8c12);
    g96 = 0x190;
    if (value_parity(campaign_round_node_cursor) != 0) {
        resource_load_record((dos_uint)(g73c + 0x101e));
        gfx_copy_rect(8, 0xc8, ui_gfx_shadow_a, 0);
    }
    n = *(q = record_table_level4_ptr());
    p = q + 1;
    for (i = 0; i < n; i++, p += 3)
        if (p[2] >= 0x80) {
            j = p[0] * 2;
            w8 = p[1];
            gfx_copy_rect(j, w8 + 0xb8, (const uint8_t *)a72b2[p[2] & 0x3f], p[2] & 0x40);
        }
    icon_record_list_ptr = p;
    g94 = 0xc8;
    i = 0;
    w8 = i;
    for (; i < 0x12; i++)
        for (j = 0; j < 0x26; j++) {
            if ((c = board_records[w8]) & 7) {
                c = (c & 7) - 1;
                if (c < 6)
                    gfx_copy_rect(j * 8 + 4, i * 8 + 0xc4, (const uint8_t *)a74a2[c], 0);
            } else if (c & 0x80) {
                c = (c & 0x70) >> 4;
                if (c != 0)
                    gfx_copy_rect(j * 8 + 8, i * 8 + 0xc8, (const uint8_t *)a6f2a[c - 1], 0);
            }
            w8++;
        }
    g94 = 0x10;
    p = q + 1;
    for (i = 0; i < n; i++, p += 3)
        if (p[2] < 0x80) {
            j = p[0] * 2;
            w8 = p[1];
            gfx_copy_rect(j, w8 + 0xb8, (const uint8_t *)a72b2[p[2] & 0x3f], p[2] & 0x40);
        }
    if (gb07a != 0) {
        if (b4377[0] == board_record_index) {
            g722 = 1;
            xa[0] = b4377[1];
            xa[0] <<= 1;
            ya[0] = b4377[2];
        } else {
            g722 = 0;
        }
    }
    for (i = 0; i < g722; i++)
        gfx_blit_bitmap(xa[i], ya[i] + 0xb8, (const uint8_t *)s79bf);
    if (*record_table_root != 0)
        draw_queue_render_highlighted();
    gfx_wipe_rect(0, 0xc8, 0x140, 0x90, 0, 0x158);
    g96 = 0x190;
    board_mark_record_cells();
    draw_queue_reset();
    for (i = 0; i < 6; i++)
        if (board_record_index + 1 == b437a[i]) {
            gfx_copy_rect((w8 = b4380[i]) * 2, (j = b4386[i]) + 0xb8, (const uint8_t *)s7400[0], 0);
            draw_queue_append((dos_char)(i + 1), w8, j, 8, 0x10);
        }
    /* PORT: historical `((unsigned char far *)board_records)[...]` -- board_records
     * is signed dos_char*; the explicit unsigned reinterpretation must be kept so
     * a byte with the high bit set compares/widens as 0..255, not -128..-1. */
    if (((dos_uchar *)board_records)[0x3e7] == board_record_index + 1) {
        gfx_copy_rect((w8 = ((dos_uchar *)board_records)[0x3e5]) * 2, (j = ((dos_uchar *)board_records)[0x3e6]) + 0xb8, (const uint8_t *)s735e[0], 0);
        draw_queue_append(7, w8, j, 8, 0x10);
    }
    p = (dos_uchar *)(record_table_root + *record_table_root * 4 + 1);
    n = *p;
    i = 0;
    p++;
    for (; i < n; i++, p += p[0]) {
        j = p[2];
        w8 = p[3];
        if ((w4 = p[1]) > 2) {
            draw_queue_append((dos_char)(i + 8), j, w8, 4, 8);
        } else if (w4 == 0) {
            if (p[4] != 0)
                gfx_copy_rect(j * 2, w8 + 0xb8, (const uint8_t *)s9b6e[0], 0);
            else
                gfx_copy_rect(j * 2, w8 + 0xb8, (const uint8_t *)s9ae0[0], 0);
            draw_queue_append((dos_char)(i + 8), j + 6, w8 + 3, 2, 6);
        } else if (w4 == 1) {
            if (p[4] != 0)
                gfx_copy_rect(j * 2, w8 + 0xb8, (const uint8_t *)s9a5c[0], 0);
            else
                gfx_copy_rect(j * 2, w8 + 0xb8, (const uint8_t *)s99da[0], 0);
            draw_queue_append((dos_char)(i + 8), j + 3, w8, 7, 7);
        } else {
            gfx_copy_rect(j * 2, w8 + 0xb8, (const uint8_t *)s9c50[0], 0);
            draw_queue_append((dos_char)(i + 8), j, w8, 8, 0x10);
        }
    }
    n = *(g96e6 = p);
    i = 0;
    p++;
    for (; i < n; i++, p += 3) {
        j = p[0] * 2;
        w8 = p[1];
        gfx_copy_rect(j, w8 + 0xb8, (const uint8_t *)s6e88[0], 0);
        gfx_copy_rect(j + 4, w8 + 0xb8, (const uint8_t *)a893c[p[2]], 0);
        draw_queue_append((dos_char)(p[2] + 0x20), p[0] + 1, w8 + 4, 6, 0xa);
    }
    n = *(g96ea = p);
    i = 0;
    p++;
    for (; i < n; i++, p += 0xc) {
        f2986(p);
        w2 = p[0] / 4 + (p[1] / 8 - 1) * 0x26 - 1;
        for (w8 = 0; w8 < 6; w8++, w2++)
            board_records[w2] = board_records[w2 + 0x26] = 7;
    }
    if (*(g40d0 = (dos_char *)p) != 0)
        sprite_table_queue_draws();
    g96 = 0x9f;
}


/* ---- F_31C4 (original code at 0x31C4) ---- */
/* F_31C4 -- scroll the board into view, redraw the frame, scroll it out.
   si is the row cursor in both loops; ui_gfx_blob is copied into rect_queue_write_ptr as a far
   pointer (les bx / mov es / mov bx). */
void board_scroll_transition(void)
{
    dos_int y;

    if (g072e <= 8) {
        rect_queue_write_ptr = ui_gfx_blob;
        for (y = 0x10; y < 0x14; y++) {
            timer_deadline_arm(0x30);
            g072e = y;
            board_redraw_view();
            sprite_draw_cursor();
            rect_queue_flush();
            timer_deadline_wait();
        }
        timer_wait_ticks(0x30);
    }
    gbc = 0;
    sound_stop_reset();
    puzzle_run();
    board_redraw_paint();
    gfx_wipe_rect(8, 0xc8, 0x130, 0x90, 8, 0x10);
    board_actors_draw(0);
    sprite_draw_cursor();
    gfx_box(8, 0x10, 0x130, 0x90);
    hud_panel_open();
    gbc = 1;
    rect_queue_write_ptr = ui_gfx_blob;
    if (g072e == 0x13) {
        for (y = 0x13; y >= 0x10; y--) {
            timer_deadline_arm(0x30);
            g072e = y;
            board_redraw_view();
            sprite_draw_cursor();
            rect_queue_flush();
            timer_deadline_wait();
        }
        g072e = 0;
        timer_wait_ticks(0x30);
    }
}


/* ---- F_329F (original code at 0x329F) ---- */
void board_record_index_select(void)
{
    board_records = (dos_char *)&g43b4[board_record_index];
    board_redraw_paint();
    rect_queue_write_ptr = ui_gfx_blob;
    gfx_wipe_rect(8, 0xc8, 0x130, 0x90, 8, 0x10);
    g40ce = gbc = 0;
    board_actors_draw(0);
    gbc = 1;
}


/* ---- F_32FA (original code at 0x32FA) ---- */
/* K&R implicit-int; every call site (src/BOARD.C:699, src/BOARD.C's own
 * board_run_unit_script) discards the return value, so `return 0;` below
 * is a compile-cleanliness addition, not a semantic one. */
dos_int f32fa(dos_int a)
{
    dos_int i;
    dos_int j;
    dos_int x, y, n;
    dos_uchar *p;
    p = (dos_uchar *)(record_table_root + a * 4 + 1);
    p[3] = (p[3] + 4) & 7;
    n = p[2] + 2;
    x = p[0];
    y = p[1];
    j = (x + 2) / 4 + ((y + 4) / 8 - 2) * 38 - 1;
    for (i = 0; i < n; i++, j++)
        board_records[j] ^= 16;
    return 0;
}


/* ---- F_338A (original code at 0x338A) ---- */
/* F_338A -- dispatch one scripted event stream and animate its unit. */
void board_run_unit_script(dos_uchar *s)
{
    dos_int w, v, u, i, n, k;
    dos_uchar c;
    dos_uchar *r;
    dos_uchar *q;
    dos_int x, y;

    n = *s;
    if (s[1] < 2) {
        x = s[2];
        x <<= 1;
        y = s[3];
        if (s[1] != 0) {
            x = s[2];
            x <<= 1;
            y = s[3];
            gfx_wipe_rect(x, y + 0x148, 0x18, 8, x, y + 0xb8);
            g96 = 0x167;
            if (s[4] ^= 1)
                gfx_copy_rect(x, y + 0xb8, (const uint8_t *)s9a5c[0], 0);
            else
                gfx_copy_rect(x, y + 0xb8, (const uint8_t *)s99da[0], 0);
            g96 = 0x9f;
            gfx_wipe_rect(x, y + 0xb8, 0x18, 8, x, y);
        } else {
            x = s[2];
            x <<= 1;
            y = s[3];
            gfx_wipe_rect(x, y + 0x148, 0x18, 9, x, y + 0xb8);
            g96 = 0x167;
            if (s[4] ^= 1)
                gfx_copy_rect(x, y + 0xb8, (const uint8_t *)s9b6e[0], 0);
            else
                gfx_copy_rect(x, y + 0xb8, (const uint8_t *)s9ae0[0], 0);
            g96 = 0x9f;
            gfx_wipe_rect(x, y + 0xb8, 0x18, 9, x, y);
        }
        stream_control_block_arm(8);
    } else if (s[1] == 2)
        stream_control_block_arm(0x16);
    s += i = 5;
    for (; i < n; i++, s++) {
        c = *s;
        if ((c & 0x80) == 0) {
            if (c & 0x10)
                f32fa(c & 0xf);
            else if (c & 0x40)
                sprite_record_adjust_draw(c & 0xf);
            else
                f250c(c);
        } else if ((c & 0x30) == 0) {
            i++;
            s++;
            r = g43b4[*s].bytes + (c & 0x7f) * 3 + 0x2ac;
            c = *r;
            *r &= 0xf0;
            *r ^= 0x20;
            if (c & 0x80) {
                if (c & 0x20)
                    r[2] -= 0x30;
                else
                    r[2] += 0x30;
            } else {
                if (c & 0x20)
                    r[1] -= 0x18;
                else
                    r[1] += 0x18;
            }
        } else if (c & 0x10) {
            i++;
            s++;
            r = (q = g43b4[*s].bytes) + (c & 0xf) * 4 + 0x2cb;
            /* r[3] = -r[3], negated at WORD width without an extension: every
               C spelling (28 standalone forms, 8 in context, 2026-09-21,
               docs/history/probes/neg-ax-forms.C) folds a byte-lvalue negation
               to `neg al`, and every route to `neg ax` (int/static/register
               temp) materialises `mov ah,0` plus a spill/reload.  The original
               relies on AH still being zero from the `and ax,0Fh` three
               statements earlier, which only the author knew: hand-written
               inline asm inside this otherwise compiled function.
               PORT: `neg ax` with AH==0 computes -(0:AL) as a 16-bit value;
               truncated back to AL on store, that is bit-for-bit identical to
               an 8-bit `neg al` regardless of AH's value, so a plain byte
               negate reproduces the stored result exactly. */
            r[3] = (dos_uchar)(-(dos_int)r[3]);
            c = r[2] + 2;
            w = r[0];
            v = r[1];
            u = (w + 2) / 4 + ((v + 4) / 8 - 2) * 0x26 - 1;
            for (k = 0; c > k; k++, u++)
                q[u] ^= 0x10;
        } else if (c & 0x20) {
            i++;
            s++;
            c = *s;
            actor_state_table[c].flag = 0;
        }
    }
}


/* ---- F_36F0 (original code at 0x36F0) ---- */
/* F_36F0 -- advance every queued 12-byte move record by one step: either
   finish the move (swap the from/to cells, repaint, re-mark the board) or
   redraw the unit at its current cell. */
void board_advance_unit_moves(dos_int a)
{
    dos_int n;
    dos_int i;
    dos_uchar *p;
    dos_uchar s;
    dos_uchar cx;
    dos_uchar cy;
    dos_int u, k;

    g96 = 0x190;
    stream_control_block_arm(0xa);
    a++;
    n = *g96ea;
    p = g96ea + 1;
    for (i = 0; i < n; i++, p += 0xc) {
        s = p[4];
        cx = p[0];
        cy = p[1];
        if (p[s + 5] != a) {
            if (s != 0) {
                p[4] = 0;
                f2986(p);
                gfx_wipe_rect(cx * 2, cy + 0xb8, 0x38, 0x10, cx * 2, cy);
            }
        } else {
            p[4] = ++s;
            if (s == 5 || p[s + 5] == 0) {
                p[4] = 0;
                u = p[0] / 4 + (p[1] / 8 - 1) * 0x26 - 1;
                for (k = 0; k < 6; k++, u++)
                    board_records[u] = board_records[u + 0x26] = 0;
                p[0] = p[2];
                p[1] = p[3];
                p[2] = cx;
                p[3] = cy;
                gfx_wipe_rect(cx * 2, cy + 0x148, 0x38, 0x10, cx * 2, cy);
                gfx_wipe_rect(cx * 2, cy + 0x148, 0x38, 0x10, cx * 2, cy + 0xb8);
                f2986(p);
                cx = p[0];
                cy = p[1];
                gfx_wipe_rect(cx * 2, cy + 0xb8, 0x38, 0x10, cx * 2, cy);
                u = cx / 4 + (cy / 8 - 1) * 0x26 - 1;
                for (k = 0; k < 6; k++, u++)
                    board_records[u] = board_records[u + 0x26] = 7;
            } else {
                f2986(p);
                gfx_wipe_rect(cx * 2, cy + 0xb8, 0x38, 0x10, cx * 2, cy);
            }
        }
    }
    g96 = 0x9f;
}


/* ---- F_3986 (original code at 0x3986) ---- */
void board_record_complete(void)
{
    dos_int si, di, w, h;

    g73e = board_record_index;
    g96 = 0x190;
    board_actors_draw(0xb8);
    g96 = 0x9f;

    if (cursor_x < 8) {
        si = 8;
        w = 0x28;
    } else if (cursor_x + 0x27 > 0x137) {
        si = cursor_x;
        w = 0x138 - cursor_x;
    } else {
        si = cursor_x;
        w = 0x28;
    }

    if (cursor_y < 0x10) {
        di = 0x10;
        h = 0x28;
    } else if (cursor_y + 0x27 > 0x9f) {
        di = cursor_y;
        h = 0xa0 - cursor_y;
    } else {
        di = cursor_y;
        h = 0x28;
    }

    anim_step_loop(si, di + 0xb8, w, h, si, di);

    slot_table[current_slot].value++;
    energy_set(slot_table[current_slot].state = 4);
    if (confirm_quit_dialog())
        game_abort(GAME_RETURN_MAP);
}
