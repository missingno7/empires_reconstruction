/* game.c -- src/GAME.C: turn loop, terrain, level driver, boot and campaign
 * flow.  One translation unit; the sections below were the separate member
 * sources of grouped module C_3A75_4A93 and keep their original ids.
 *
 * PORT (per docs/portable/tu-port-agent-brief.md, this file's specific
 * instructions):
 *  - `main` -> `game_main` (kept void; the historical function never sets
 *    AX before its single `ret`, so no return value was ever read).
 *  - boot_init_seed_rand: `srand(biostime(0,0L))` -> `srand((dos_uint)
 *    dosio_bios_ticks())`; `dos_critical_error_install()` and `asm sti`
 *    dropped outright (no portable equivalent, DOS-only); `ui_gfx_alloc()`
 *    -> `resource_staging_init()`.
 *  - blitter_patch_variant's body becomes a no-op: the portable gfx layer
 *    already selects its driver by display_mode (docs/portable/
 *    architecture.md "Video model"), so there is nothing left to patch.
 *  - video_alloc_framebuffer's one call site is replaced by
 *    gfx_framebuffer_init(); color_lookup_tables_init(); and the palette
 *    load.  src/VIDEO.C's own video_alloc_framebuffer (read to confirm the
 *    exact rule) loads the palette only for two of the five selectors:
 *    `if (display_mode == 5) video_load_palette(g11e); else if
 *    (display_mode == 4) video_load_palette(g41e);` -- selectors 1/2/3
 *    load no palette here at all, so this is reproduced as the same
 *    two-armed if/else-if, not a ternary that would wrongly load g41e for
 *    every non-5 mode.
 *
 * generator type issues (report):
 *  - b4374 (alias g4374, DS:4374): GAME.C itself declares this address
 *    two different ways in two local scopes -- `unsigned char g4374[]`
 *    (menu_backdrop_paint, used as a >=10000-byte buffer base) and
 *    `unsigned char b4374` (level_driver_run, used as a scalar flags
 *    byte, `b4374 & 0x80`).  game_state.h picked the array reading
 *    (`dos_uchar b4374[1]`), so the buffer-base uses (bare `g4374`) need
 *    no change, but the one scalar use needs `b4374[0]` to read the byte
 *    value instead of decaying to a pointer -- done below.
 *  - g0b3ae (actor_record_table): RESOLVED -- was declared dos_uchar[1]
 *    (only one byte of unambiguous evidence from its own undimensioned
 *    historical extern) while used as the base of a 0xbb8-byte setmem/
 *    movmem span here; regenerated as dos_uchar[385] (manual override,
 *    tools/portable/datagen_overrides.json) with actor_state_table now a
 *    struct-view macro into it.  No code change needed here (setmem/
 *    movmem already just used the bare pointer).
 *  - g4374 (b4374, DS:4374): still declared dos_uchar[1] and still used as
 *    the base of a >=10000-byte movmem span (menu_backdrop_paint); same
 *    family of issue as the two resolved above, not yet fixed.  Still
 *    compiles (movmem takes an untyped pointer, no static bound check).
 */
#include "game.h"
#include "gfx_tween.h"
#include "trace.h"

/* ---- F_3A75 (original code at 0x3A75) ---- */
/* F_3A75 -- the turn loop.  Entry 13B75; the declared /24 player-query point
   13C08 is the rect_table_hit_id() probe at offset 0x93 of this function. */
dos_int turn_loop_run(void)
{
    dos_int key;                            /* bp-16 */
    dos_int obj;                            /* bp-14 */
    dos_int r;                              /* bp-12 */
    dos_int dir;                            /* bp-10 */
    dos_int oy;                             /* bp-0E */
    dos_int lastobj;                        /* bp-0C */
    dos_int probe;                          /* bp-0A */
    dos_int lastcur;                        /* bp-08 */
    dos_int blink;                          /* bp-06 */
    dos_char *p;                            /* bp-04 */
    dos_int x, d;                           /* si, di */

    g730 = g732 = 0;
    x = 0;
    g96e4 = x;
    blink = lastcur = raycast_trail_active = lastobj = x;
    gbc = key_up_released = 1;
    keyboard_chain_disable();
    EMPIRES_TRACE("turn_loop_run enter");
    for (;;) {
        { static unsigned iter; if ((++iter % 200) == 1) EMPIRES_TRACE("turn iteration %u ticks=%lu", iter, (unsigned long)timer_ticks); }
        timer_deadline_arm(0x18);
        g40ce = dir = key = 0;
        if (keyboard_poll_nonblocking()) {
            gbc = 0;
            key = keyboard_read_blocking_hotkeys();
            if (key == 0xd) hud_tab_next();
            else if (key == 0x1b) player_select_restart_confirm();
            gbc = 1;
        }
        rect_queue_write_ptr = ui_gfx_blob;
        if ((obj = rect_table_hit_id((cursor_x >> 1) + 1, cursor_y + 1, 14, 0x27)) != 0 && obj != lastobj) {
            if (obj < 7) {
                record_table_delete_compact(obj);
                obj--;
                d = b4380[obj];
                d <<= 1;
                oy = b4386[obj];
                b437a[obj] = 0;
                gb07a = puzzle_deal_pieces();
                rect_queue_write_ptr = ui_gfx_blob;
                gfx_wipe_rect(d, oy + 0x148, 0x10, 0x10, d, oy + 0xb8);
                gfx_wipe_rect(d, oy + 0xb8, 0x10, 0x10, d, oy);
                stream_control_block_arm(2);
                if (gb07a != 0) {
                    sprite_draw_cursor();
                    rect_queue_flush();
                    board_scroll_transition();
                }
            } else if (obj == 7) {
                energy_adjust(slot_table[current_slot].state = 4);
                record_table_delete_compact(obj);
                d = ((dos_uchar *)board_records)[0x3e5];
                d <<= 1;
                oy = ((dos_uchar *)board_records)[0x3e6];
                board_records[0x3e7] = 0;
                gfx_wipe_rect(d, oy + 0x148, 0x10, 0x10, d, oy + 0xb8);
                gfx_wipe_rect(d, oy + 0xb8, 0x10, 0x10, d, oy);
                stream_control_block_arm(3);
            } else if (obj < 0x20) {
                p = record_field_skip_n(obj - 8);
                if (p[1] != 2) board_run_unit_script((dos_uchar *)p);
            } else if (obj < 0x30) {
                board_advance_unit_moves(obj - 0x20);
            }
        }
        lastobj = obj;
        if (*g40d0 != 0) animated_tile_tick();
        if (raycast_trail_active != 0) board_unit_script_trigger();
        if (key_up_held != 0 && g722 != 0) {
            for (obj = 0; obj < g722; obj++) {
                if (w8bf4[obj] - 2 <= cursor_y && w8bf4[obj] + 2 >= cursor_y &&
                    w8bea[obj] <= cursor_x && w8bea[obj] + 0x10 >= cursor_x) {
                    if (g71e == 0) {
                        gbc = 0;
                        board_scan_wipe_effect(obj);
                        return 1;
                    } else {
                        gbc = 0;
                        r = anim_frame_advance(obj);
                        if (r != 0) {
                            if (r > 0) {
                                board_scan_wipe_effect(obj);
                                return 1;
                            }
                            return 0;
                        }
                        rect_queue_write_ptr = ui_gfx_blob;
                        gbc = 1;
                        goto scanned;
                    }
                }
            }
        }
scanned:
        board_redraw_view();
        sprite_table_wipe_active();
        if (raycast_trail_active != 0) sprite_slots_redraw();
        board_update_moving_records();
        if (*record_table_root != 0) draw_queue_render();
        if (*icon_record_list_ptr != 0) icon_list_animate_draw();
        if (key_up_right_held != 0) {
            EMPIRES_TRACE("turn: right held x=%d cur=(%d,%d) coll=%d", x, cursor_x, cursor_y,
                          board_collision_span_or(cursor_x + 0x21, cursor_y + 1, 0x27));
            if (x == 0 || key_up_held == 0) {
                x = 0;
                cursor_facing_left = x;
                if ((board_collision_span_or(cursor_x + 0x21, cursor_y + 1, 0x27) & 7) == 0) {
                    cursor_x += g734;
                    dir = 1;
                    if (g72e <= 8) {
                        if (++g72e > 8) g72e = 1;
                    }
                }
            }
        } else if (key_up_left_held != 0) {
            if (x == 0 || key_up_held == 0) {
                cursor_facing_left = 1;
                x = 0;
                if ((board_collision_span_or(cursor_x, cursor_y + 1, 0x27) & 7) == 0) {
                    cursor_x -= g734;
                    dir = -1;
                    if (g72e <= 8) {
                        if (++g72e > 8) g72e = 1;
                    }
                }
            }
        }
        if (key_up_held != 0 && (board_collision_span_or(cursor_x + 0x10 - (cursor_facing_left << 2), cursor_y + 1, 0x27) & 0x80) != 0) {
                if (x == 0) {
                    if (cursor_x % 8 == 0) {
                        if ((board_collision_span_or(cursor_x + 0x14, cursor_y + 1, 0x1d) & 0x80) != 0)
                            cursor_x += 4;
                        else
                            cursor_x -= 4;
                    }
                    x = 1;
                } else if ((shadow_bitmap_hit_test(cursor_x + 0xf, cursor_y - 4, 2) & 0x80) != 0) {
                    cursor_y -= 4;
                    if (++x > 2) x = 1;
                } else if ((shadow_bitmap_hit_test(cursor_x + 0xf, cursor_y - 2, 2) & 0x80) != 0) {
                    cursor_y -= 2;
                    if (++x > 2) x = 1;
                }
                g734 = 8;
                g730 = 0;
                g72e = x + 0x13;
        } else if (gb6a != 0 && x != 0) {
            if ((shadow_bitmap_hit_test(cursor_x + 0xf, cursor_y + 0x26, 2) & 0x80) != 0) {
                cursor_y += 4;
                if (++x > 2) x = 1;
                g72e = x + 0x13;
            } else {
                x = 0;
            }
        }
        if (x == 0) {
            if (g730 != 0) {
                if ((shadow_bitmap_hit_test(cursor_x + 8, cursor_y - 1, 9) & 7) == 0) {
                    cursor_y -= b740[g730];
                    g730--;
                    if (++g72e > 0xb) g72e = 0xb;
                    if (dir == 0) g72e = 0xa;
                } else {
                    g730 = 0;
                }
                goto moved;
            }
            if ((shadow_bitmap_hit_test(cursor_x + 8, cursor_y + 0x2f, 9) & 7) == 0) {
                if (g732 != 0) {
                    cursor_y += 8;
                } else {
                    g732 = 1;
                    cursor_y += 2;
                }
                g72e = (dir & 1) + 0xa;
                goto moved;
            }
            if (((probe = shadow_bitmap_hit_test(cursor_x + 8, cursor_y + 0x28, 9)) & 7) == 0) {
                cursor_y += (((cursor_y + 0x30) / 8) << 3) - (cursor_y + 0x28);
                g732 = 0;
                g72e = (dir & 1) + 0xa;
                stream_control_block_arm(0xb);
                goto moved;
            }
            if (key == 0x20) {
                if (hud_tab_get() == 1) {
                    if ((shadow_bitmap_hit_test(cursor_x + 8, cursor_y - 1, 9) & 7) == 0) {
                        stream_control_block_arm(0x10);
                        g730 = 8;
                        g72e = 9;
                        g734 = dir ? 8 : 4;
                    } else goto fell;
                } else goto fell;
                goto moved;
            }
            if (key_up_held != 0 && key_up_released != 0) {
                if ((shadow_bitmap_hit_test(cursor_x + 8, cursor_y - 1, 9) & 7) == 0) {
                    key_up_released = 0;
                    g730 = 5;
                    g72e = 9;
                    g734 = dir ? 8 : 4;
                    stream_control_block_arm(0xc);
                    goto moved;
                }
            }
fell:
            if (dir != 0) {
                if (g72e > 8) g72e = 1;
            } else {
                g72e = 0;
            }
            if (probe & 8) {
                if (probe & 0x10) {
                    if ((board_collision_span_or(cursor_x, cursor_y + 1, 0x27) & 7) == 0) {
                        cursor_x -= g734;
                        if (cursor_x <= -4) cursor_x = -0x11;
                    }
                } else {
                    if ((board_collision_span_or(cursor_x + 0x21, cursor_y + 1, 0x27) & 7) == 0) {
                        cursor_x += g734;
                        if (cursor_x >= 0x130) cursor_x = 0x131;
                    }
                }
            }
            g734 = 4;
        }
moved:
        if (key == 0x20) {
            if ((obj = hud_tab_get()) == 2 && hud_scroll_cooldown_ticks == 0) {
                    if (hud_scroll_move(-1) != -1) {
                        hud_scroll_cooldown_ticks = 0x3a;
                        blink = 0;
                        stream_control_block_arm(0);
                    } else {
                        stream_control_block_arm(0x11);
                    }
            } else if (obj == 0 && x == 0) {
                if (raycast_trail_active == 0) {
                    cursor_trail_arm();
                    stream_control_block_arm(0x14);
                } else {
                    stream_control_block_arm(0x17);
                }
            }
        }
        if (hud_scroll_cooldown_ticks == 1) hud_scroll_cooldown_ticks = 0;
        sprite_script_frame_driver();
        if (cursor_y < 0) {
            if (b43a0[board_record_index] != 0) {
                cursor_y = 0x90;
                if (b43a0[board_record_index] - 1 != board_record_index) {
                    board_record_index = b43a0[board_record_index] - 1;
                    raycast_trail_active = lastcur = lastobj = 0;
                    board_record_index_select();
                }
            } else {
                cursor_y = 0;
            }
        } else if (cursor_y > 0x90) {
            if (b43aa[board_record_index] != 0) {
                cursor_y = 0;
                if (b43aa[board_record_index] - 1 != board_record_index) {
                    board_record_index = b43aa[board_record_index] - 1;
                    raycast_trail_active = lastcur = lastobj = 0;
                    board_record_index_select();
                }
            } else {
                cursor_y = 0x90;
            }
        }
        if (cursor_x < -0x10) {
            if (b438c[board_record_index] != 0) {
                cursor_x = 0x120;
                if (b438c[board_record_index] - 1 != board_record_index) {
                    board_record_index = b438c[board_record_index] - 1;
                    raycast_trail_active = lastcur = lastobj = 0;
                    board_record_index_select();
                }
            } else {
                cursor_x = -0x10;
            }
        } else if (cursor_x > 0x130) {
            if (b4396[board_record_index] != 0) {
                cursor_x = 0;
                if (b4396[board_record_index] - 1 != board_record_index) {
                    board_record_index = b4396[board_record_index] - 1;
                    raycast_trail_active = lastcur = lastobj = 0;
                    board_record_index_select();
                }
            } else {
                cursor_x = 0x130;
            }
        }
        if (raycast_trail_active != 0) board_raycast_step();
        gfx_tween_tag = GFX_TWEEN_TAG_PLAYER;   /* frame interpolation: the player draws below (observer tag, see gfx_tween.h) */
        if (blink != 0) {
            blink--;
            if (blink > 0x1a) {
                gfx_copy_rect(cursor_x, cursor_y, (const uint8_t *)(resource_stripe_table + 0x39ec), cursor_facing_left);
            } else if (blink & 1) {
                gfx_copy_rect(cursor_x, cursor_y, (const uint8_t *)(resource_stripe_table + g72e * 0x2a2), cursor_facing_left);
            }
        } else if (hud_scroll_cooldown_ticks != 0) {
            gfx_copy_rect(cursor_x, cursor_y, str96ee, 0);
            hud_scroll_cooldown_ticks--;
            gfx_copy_rect(cursor_x, cursor_y, (const uint8_t *)(resource_stripe_table + g72e * 0x2a2), cursor_facing_left);
        } else {
            if (g40ce != 0 && lastcur != g40ce) {
            gfx_copy_rect(cursor_x, cursor_y, (const uint8_t *)(resource_stripe_table + 0x39ec), cursor_facing_left);
            gfx_tween_tag = 0;
            blink = 0x1e;
            stream_control_block_arm(1);
            rect_queue_flush();
            gbc = 0;
            if (energy_adjust(-1) == 0) {
                board_record_complete();
                return 0;
            }
            slot_table[current_slot].state = (dos_char)energy_adjust(0);
            gbc = 1;
            x = 0;
            lastcur = g40ce;
            goto tail;
            } else {
            gfx_copy_rect(cursor_x, cursor_y, (const uint8_t *)(resource_stripe_table + g72e * 0x2a2), cursor_facing_left);
            }
            lastcur = g40ce;
        }
        gfx_tween_tag = 0;
        rect_queue_flush();
tail:
        timer_deadline_wait();
        if (g71e != 0 && g96e4 == 0 && cursor_x > 0xbe) {
            roundend_flash_panel_icons();
            g96e4 = 1;
        }
    }
    /* Historically unreachable (the for(;;) above only ever exits through
     * an internal `return`).  PORT: dropped rather than kept as dead code
     * under an unreliable pragma -- `#pragma warning(disable:4702)` does
     * not suppress MSVC's unreachable-code diagnostic here, and the
     * brief's zero-warnings rule leaves no other option; this has no
     * behavioral effect either way since the statements below could never
     * execute: `gbc = 0; return 1;`. */
}


/* ---- F_4517 (original code at 0x4517) ---- */
void board_terrain_resources_load(dos_int di)
{
    dos_char *d;
    dos_int i;

    if (g73c == di)
        return;
    g73c = di;
    resource_load_record(di + 0x1015);
    d = (dos_char *)(ui_gfx_shadow_a + 2);
    for (i = 0; i < 5; i++, d += 0x31b)
        movmem(d, s79bf + i * 0x319, 0x319);
    /* PORT: a74a2 is generated as `dos_char a74a2[6][187]` (2D, matching
     * BOARD.C's own `char a74a2[][0xbb]` extern); this file's OWN local
     * extern declared it flat (`char a74a2[];`, GAME.C:407) and indexed by
     * hand (`a74a2 + i*0xbb`) -- rewritten as row indexing a74a2[i], the
     * generated type. */
    for (i = 0; i < 6; i++, d += 0xbd)
        movmem(d, a74a2[i], 0xbb);
    movmem(d, s8c12, 0xad2);
    resource_load_record_into(di + 0x1019, (uint8_t *)g99d6);
    i = 0;
    /* PORT: `((unsigned far *)g99d6)[i]` reinterprets a byte buffer through
     * a wider pointer type -- dos_types.h: "never reinterpret buffers
     * through wider pointer types"; use the little-endian word reader
     * instead (the historical table is an array of 16-bit offsets). */
    for (; i < g724[g73c]; i++)
        a72b2[i] = g99d6 + dos_rd16((const uint8_t *)(g99d6 + i * 2)) + 2;
    for (; i < 0x28; i++)
        a72b2[i] = a72b2[0];
}


/* ---- F_462E (original code at 0x462E) ---- */
/* F_462E -- paint the title/menu backdrop for the current mode.  si is the
   mode, di the row offset chosen by the two-term disjunction at 4698. */
void menu_backdrop_paint(void)
{
    dos_int m, d;

    menu_resources_load();
    if (value_parity(campaign_round_node_cursor))
        m = 0x14;
    else
        m = campaign_round_node_cursor / 2;
    if (m == 0x15)
        resource_load_record(0x42);
    else
        resource_load_record(m + 0x1000);
    if (m == 0x14) {
        movmem(ui_gfx_shadow_a, g4374, 0x2750);
        setmem(actor_record_table, 0xbb8, 0);
    } else {
        if (slot_is_new_game() == 0 || m > 0x14)
            d = 0;
        else
            d = 0x330c;
        movmem(ui_gfx_shadow_a + d + 2, g4374, 0x2750);
        movmem(ui_gfx_shadow_a + d + 0x2754, actor_record_table, 0xbb8);
    }
    if (m < 0x14)
        board_terrain_resources_load(g4374[0] & 0x7f);
    board_records = (dos_char *)&g43b4[board_record_index = 0];
}


/* ---- F_4713 (original code at 0x4713) ---- */
/* F_4713 -- the level driver: set the map geometry, build the view, then run
   the turn loop in F_3A75 and hand back its result.  No frame at all (no
   parameters, no locals, one register variable), which is what -k- gives. */
dos_int level_driver_run(void)
{
    EMPIRES_TRACE("level_driver_run node=%d", campaign_round_node_cursor);
    dos_int r;

    gfx_tween_scene_reset();
    menu_backdrop_paint();
    menu_list_source_set_default();
    cursor_x = b4375;
    cursor_x <<= 1;
    cursor_x = ((cursor_x + 3) >> 2) << 2;
    cursor_y = b4376;
    /* PORT: b4374 is generated as dos_uchar[1] (array); the scalar byte
     * read needs the explicit [0] -- see file header "generator type
     * issues". */
    if (b4374[0] & 0x80) cursor_facing_left = 1;
    else cursor_facing_left = 0;
    hud_scroll_cooldown_ticks = gb07a = g72e = 0;
    if (value_parity(campaign_round_node_cursor)) {
        g722 = 3;
        /* PORT: g8bea/g8bf4 are still #define aliases for the WHOLE xa/ya
         * arrays (`#define g8bea xa`), not scalars, so they cannot appear
         * on the left of `=` directly -- xa[0]/ya[0] spell the same
         * DS:8BEA/DS:8BF4 storage the historical scalar assignment
         * targeted.  g8bec/g8bee/g8bf6/g8bf8 (unlike g8bea/g8bf4) ARE now
         * proper element-alias macros (xa[1]/xa[2]/ya[1]/ya[2], following
         * the xa[5]/ya[5] regeneration -- see file header), so they are
         * used bare below, matching the historical spelling exactly. */
        xa[0] = g8bec = g8bee = 0xf4;
        ya[0] = 0x12;
        g8bf6 = 0x42;
        g8bf8 = 0x72;
        g71e = 1;
        board_redraw_paint();
        roundend_round_setup(campaign_round_node_cursor >> 1);
    } else {
        g722 = g71e = 0;
        puzzle_clear_grid();
        hud_scroll_reset();
        board_redraw_paint();
    }
    if (g73e == 0) {
        gfx_color_select(1);
        gfx_clear_rect(8, 0x10, 0x130, 0x90);
    }
    anim_step_loop(8, 0xc8, 0x130, 0x90, 8, 0x10);
    board_actors_draw(0);
    sprite_draw_cursor();
    hud_panel_open();
    gfx_box(0, 0, 0x140, 0xc8);
    if (tick_div8() != 4 && (campaign_round_node_cursor & 7) == 0) {
        tutorial_hint_dialog_show(0);
        tutorial_hint_dialog_show(1);
        if (tick_div8() == 1) tutorial_hint_dialog_show(2);
        else if (tick_div8() == 2) tutorial_hint_dialog_show(3);
    }
    snd_flag2 = 1;
    if (value_parity(campaign_round_node_cursor) == 0) {
        if (tick_div8() != 4)
            resource_record_cache_reset((tick_div8() << 2) + (campaign_round_node_cursor & 2) + 0x1073);
        else
            resource_record_cache_reset((campaign_node_index() << 2) + 0x1073);
    }
    r = turn_loop_run();
    if (r != 0 && campaign_round_node_cursor == 0x27) {
        r = level_play_chapter();
        campaign_round_node_cursor = 0x27;
    }
    return r;
}


/* ---- F_48BE (original code at 0x48BE) ---- */
/* F_48BE -- historically patched the blitter at IP 0x039C in place with the
 * variant record for the current display mode.  PORT: no-op -- the
 * portable gfx layer selects its driver by display_mode directly (see
 * docs/portable/architecture.md "Video model"), so there is no blitter
 * jump table left to patch. */
void blitter_patch_variant(void)
{
}


/* ---- F_490D (original code at 0x490D) ---- */
/* F_490D -- boot init.  Entry 14A0D, 54 bytes.  A straight-line sequence
   of twelve calls, no branches. */
void boot_init_seed_rand(void)
{
    srand((dos_uint)dosio_bios_ticks());   /* PORT: biostime(0,0L) -> dosio_bios_ticks() (rule 4) */
    timer_irq_install();
    /* PORT: dos_critical_error_install() dropped -- DOS-only, no portable
     * equivalent (tu-porting-rules.md sec 4). */
    resource_staging_init();               /* PORT: ui_gfx_alloc() -> resource_staging_init() */
    blitter_patch_variant();
    /* PORT: video_alloc_framebuffer() -> gfx_framebuffer_init() +
     * color_lookup_tables_init() + the palette load src/VIDEO.C's own
     * video_alloc_framebuffer performs (see file header comment for the
     * exact two-armed rule, verified against src/VIDEO.C). */
    gfx_framebuffer_init();
    color_lookup_tables_init();
    if (display_mode == 5)
        video_load_palette(g11e);
    else if (display_mode == 4)
        video_load_palette(g41e);
    player_record_load_publish();
    keyboard_irq_install();
    /* PORT: `asm sti` dropped -- DOS interrupt-flag mechanics, no portable
     * equivalent (tu-porting-rules.md sec 5's `__sti__()`/`enable()` row). */
    resource_stripe_table_load();
    sprite_load_boot_sheets();
    sprite_sheet_select(0);
}


/* ---- F_4943 (original code at 0x4943) ---- */
void campaign_chapter_advance(dos_int i)
{
    slot_table[current_slot].resume_round = (dos_char)(i + 1);
    campaign_round_node_cursor = slot_table[current_slot].round_progress[i] * 2 + i * 8;
    g73e = -1;
    while (1) {
        if (value_parity(campaign_round_node_cursor)) {
            if (!level_driver_run())
                campaign_round_node_cursor -= 2;
            else {
                slot_table[current_slot].round_progress[i]++;
                if ((campaign_round_node_cursor & 7) == 7)
                    break;
            }
        } else {
            while (!level_driver_run())
                ;
        }
        campaign_round_node_cursor++;
    }
}


/* ---- F_49E3 (original code at 0x49E3) ---- */
void game_shutdown(void)
{
    sound_stop_reset();
    sound_voices_reset();
    keyboard_irq_restore();
    timer_irq_restore();
}


/* ---- F_49F0 (original code at 0x49F0) ---- */
/* F_49F0 -- "THE GAME": the top-level sequencer that calls the intro driver
   intro_run_chapter (157C6) and then runs the per-level record sequence.  Entry 14AF0,
   163 bytes, 16 activations per cold boot. */
void game_run(void)
{
    dos_int n;                              /* bp-2 */
    dos_int s, d;                           /* si, di */

    if (intro_run_chapter() == -1) return;
    hud_arena_init();
    d = setjmp(game_abort_jmpbuf);   /* enum GameRunResult, see game_flow.h */
    s = -1;
    ui_overlay_reset();
    sound_request_count_clear();
    puzzle_free_resources();
    if (d == GAME_EXIT) return;
    if (d < GAME_RETURN_MAP) {   /* GAME_FRESH or GAME_RESTART */
        if (slot_menu_run() == -1) return;
        energy_set(slot_table[current_slot].state);
        if ((n = slot_table[current_slot].resume_round) != 0) {
            n--;
            campaign_chapter_advance(s = n);
        }
    }
    while (s != 4) {
        s = player_select_run(s);
        campaign_chapter_advance(s);
    }
    slot_archive_and_delete();
}


/* Phase 11: the only longjmp in the port (see game_flow.h). */
void game_abort(enum GameRunResult how)
{
    longjmp(game_abort_jmpbuf, (int)how);
}

/* ---- F_4A93 (original code at 0x4A93) ---- */
/* F_4A93 -- main() (renamed game_main() -- rule: keep historical function
   names except main in GAME.C).  Entry 14B93, 20 bytes.  A straight-line
   sequence of five calls: video_mode_select() gates whether the rest ever
   runs (init failure -> immediate return with no side effects on the four
   other calls), then boot_init_seed_rand (boot init), game_run (the game),
   game_shutdown (shutdown), video_set_text_mode (exit).  No MOV sets ax
   before the single ret historically, so no "return <value>" appears; kept
   void here. */
void game_main(void)
{
    if (video_mode_select()) {
        boot_init_seed_rand();
        game_run();
        game_shutdown();
        video_set_text_mode();
    }
}
