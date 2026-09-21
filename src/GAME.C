/* src/GAME.C: Turn loop, terrain, level driver, boot and campaign flow.
   One translation unit; the sections below were the separate member
   sources of grouped module C_3A75_4A93 and keep their original ids. */

struct C470 { char pad[0x15]; char b15; char rest[5]; };

extern int keyboard_poll_nonblocking(), keyboard_read_blocking_hotkeys(), hud_tab_next(), rect_table_hit_id();
extern void timer_deadline_arm();
extern void player_select_restart_confirm();
extern void keyboard_chain_disable(void);
extern int puzzle_deal_pieces();extern void gfx_wipe_rect();
extern void record_table_delete_compact();
extern void stream_control_block_arm();
extern void rect_queue_flush(void);
extern void board_scroll_transition();
extern void sprite_draw_cursor(void);
extern int energy_adjust(), animated_tile_tick(), anim_frame_advance();
extern void board_run_unit_script();
extern void board_unit_script_trigger(void);
extern void board_scan_wipe_effect(int);
extern void board_advance_unit_moves(int a);
extern int draw_queue_render(), icon_list_animate_draw(), board_collision_span_or();extern void sprite_table_wipe_active();
extern void board_redraw_view();
extern void sprite_slots_redraw();
extern void board_update_moving_records();
extern int shadow_bitmap_hit_test(), hud_tab_get(), hud_scroll_move();extern void sprite_script_frame_driver();
extern void cursor_trail_arm(void);
extern void board_record_index_select(void);
extern void board_raycast_step(void);
extern void roundend_flash_panel_icons(void);
extern void timer_deadline_wait(void);
extern void board_record_complete();
extern char far *record_field_skip_n();

extern int g0bc, g72c, g72e, g730, g732, g734, cursor_x, cursor_y, g73a;
extern int g8fe, key_up_held, gb6a, key_up_left_held, key_up_right_held, key_up_released, g71e, g722, current_slot;
extern int g40ce, g96e4, gb07a, board_record_index;
extern char far *rect_queue_write_ptr;                 /* the edge cursor */
#include "LAYOUT.H"
#include "VIDEO.H"
extern char far *ui_gfx_blob;
extern char far *board_records;                 /* vram */
extern char far *record_table_root;
extern unsigned char far *icon_record_list_ptr;
extern char far *g40d0;                 /* objtab */
extern char far *resource_stripe_table;                 /* sprite base */
extern char str96ee[];
extern unsigned char b740[];
extern unsigned char b4380[], b4386[];
extern unsigned char b437a[];
extern char b438c[], b4396[], b43a0[], b43aa[];
extern int w8bea[], w8bf4[];
extern struct C470 c470[];
extern int resource_load_record();
extern void movmem();
extern char far *ui_gfx_shadow_a;                 /* DS:C5C6 offset, DS:C5C8 segment */
extern int campaign_round_node_cursor;
extern int value_parity();
extern int g73e;

/* ---- F_3A75 (original code at 0x3A75) ---- */
/* F_3A75 -- the turn loop.  Entry 13B75; the declared /24 player-query point
   13C08 is the rect_table_hit_id() probe at offset 0x93 of this function. */
int turn_loop_run()
{
    int key;                            /* bp-16 */
    int obj;                            /* bp-14 */
    int r;                              /* bp-12 */
    int dir;                            /* bp-10 */
    int oy;                             /* bp-0E */
    int lastobj;                        /* bp-0C */
    int probe;                          /* bp-0A */
    int lastcur;                        /* bp-08 */
    int blink;                          /* bp-06 */
    char far *p;                        /* bp-04 */
    register int x, d;                  /* si, di */

    g730 = g732 = 0;
    x = 0;
    g96e4 = x;
    blink = lastcur = g8fe = lastobj = x;
    g0bc = key_up_released = 1;
    keyboard_chain_disable();
    for (;;) {
        timer_deadline_arm(0x18);
        g40ce = dir = key = 0;
        if (keyboard_poll_nonblocking()) {
            g0bc = 0;
            key = keyboard_read_blocking_hotkeys();
            if (key == 0xd) hud_tab_next();
            else if (key == 0x1b) player_select_restart_confirm();
            g0bc = 1;
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
                energy_adjust(c470[current_slot].b15 = 4);
                record_table_delete_compact(obj);
                d = ((unsigned char far *)board_records)[0x3e5];
                d <<= 1;
                oy = ((unsigned char far *)board_records)[0x3e6];
                board_records[0x3e7] = 0;
                gfx_wipe_rect(d, oy + 0x148, 0x10, 0x10, d, oy + 0xb8);
                gfx_wipe_rect(d, oy + 0xb8, 0x10, 0x10, d, oy);
                stream_control_block_arm(3);
            } else if (obj < 0x20) {
                p = record_field_skip_n(obj - 8);
                if (p[1] != 2) board_run_unit_script(p);
            } else if (obj < 0x30) {
                board_advance_unit_moves(obj - 0x20);
            }
        }
        lastobj = obj;
        if (*g40d0 != 0) animated_tile_tick();
        if (g8fe != 0) board_unit_script_trigger();
        if (key_up_held != 0 && g722 != 0) {
            for (obj = 0; obj < g722; obj++) {
                if (w8bf4[obj] - 2 <= cursor_y && w8bf4[obj] + 2 >= cursor_y &&
                    w8bea[obj] <= cursor_x && w8bea[obj] + 0x10 >= cursor_x) {
                    if (g71e == 0) {
                        g0bc = 0;
                        board_scan_wipe_effect(obj);
                        return 1;
                    } else {
                        g0bc = 0;
                        r = anim_frame_advance(obj);
                        if (r != 0) {
                            if (r > 0) {
                                board_scan_wipe_effect(obj);
                                return 1;
                            }
                            return 0;
                        }
                        rect_queue_write_ptr = ui_gfx_blob;
                        g0bc = 1;
                        goto scanned;
                    }
                }
            }
        }
scanned:
        board_redraw_view();
        sprite_table_wipe_active();
        if (g8fe != 0) sprite_slots_redraw();
        board_update_moving_records();
        if (*record_table_root != 0) draw_queue_render();
        if (*icon_record_list_ptr != 0) icon_list_animate_draw();
        if (key_up_right_held != 0) {
            if (x == 0 || key_up_held == 0) {
                x = 0;
                g73a = x;
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
                g73a = 1;
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
        if (key_up_held != 0 && (board_collision_span_or(cursor_x + 0x10 - (g73a << 2), cursor_y + 1, 0x27) & 0x80) != 0) {
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
            if ((obj = hud_tab_get()) == 2 && g72c == 0) {
                    if (hud_scroll_move(-1) != -1) {
                        g72c = 0x3a;
                        blink = 0;
                        stream_control_block_arm(0);
                    } else {
                        stream_control_block_arm(0x11);
                    }
            } else if (obj == 0 && x == 0) {
                if (g8fe == 0) {
                    cursor_trail_arm();
                    stream_control_block_arm(0x14);
                } else {
                    stream_control_block_arm(0x17);
                }
            }
        }
        if (g72c == 1) g72c = 0;
        sprite_script_frame_driver();
        if (cursor_y < 0) {
            if (b43a0[board_record_index] != 0) {
                cursor_y = 0x90;
                if (b43a0[board_record_index] - 1 != board_record_index) {
                    board_record_index = b43a0[board_record_index] - 1;
                    g8fe = lastcur = lastobj = 0;
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
                    g8fe = lastcur = lastobj = 0;
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
                    g8fe = lastcur = lastobj = 0;
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
                    g8fe = lastcur = lastobj = 0;
                    board_record_index_select();
                }
            } else {
                cursor_x = 0x130;
            }
        }
        if (g8fe != 0) board_raycast_step();
        if (blink != 0) {
            blink--;
            if (blink > 0x1a) {
                gfx_copy_rect(cursor_x, cursor_y, resource_stripe_table + 0x39ec, g73a);
            } else if (blink & 1) {
                gfx_copy_rect(cursor_x, cursor_y, resource_stripe_table + g72e * 0x2a2, g73a);
            }
        } else if (g72c != 0) {
            gfx_copy_rect(cursor_x, cursor_y, (char far *)str96ee, 0);
            g72c--;
            gfx_copy_rect(cursor_x, cursor_y, resource_stripe_table + g72e * 0x2a2, g73a);
        } else {
            if (g40ce != 0 && lastcur != g40ce) {
            gfx_copy_rect(cursor_x, cursor_y, resource_stripe_table + 0x39ec, g73a);
            blink = 0x1e;
            stream_control_block_arm(1);
            rect_queue_flush();
            g0bc = 0;
            if (energy_adjust(-1) == 0) {
                board_record_complete();
                return 0;
            }
            c470[current_slot].b15 = energy_adjust(0);
            g0bc = 1;
            x = 0;
            lastcur = g40ce;
            goto tail;
            } else {
            gfx_copy_rect(cursor_x, cursor_y, resource_stripe_table + g72e * 0x2a2, g73a);
            }
            lastcur = g40ce;
        }
        rect_queue_flush();
tail:
        timer_deadline_wait();
        if (g71e != 0 && g96e4 == 0 && cursor_x > 0xbe) {
            roundend_flash_panel_icons();
            g96e4 = 1;
        }
    }
    g0bc = 0;
    return 1;
}


/* ---- F_4517 (original code at 0x4517) ---- */
extern void resource_load_record_into();
extern int g73c;
extern int g724[];
extern char s79bf[];
extern char a74a2[];
extern char s8c12[];
extern char far *g99d6;
extern char far *a72b2[];

void board_terrain_resources_load(di)
int di;
{
    char far *d;
    register int i;

    if (g73c == di)
        return;
    g73c = di;
    resource_load_record(di + 0x1015);
    d = ui_gfx_shadow_a + 2;
    for (i = 0; i < 5; i++, d += 0x31b)
        movmem(d, s79bf + i * 0x319, 0x319);
    for (i = 0; i < 6; i++, d += 0xbd)
        movmem(d, a74a2 + i * 0xbb, 0xbb);
    movmem(d, s8c12, 0xad2);
    resource_load_record_into(di + 0x1019, g99d6);
    i = 0;
    for (; i < g724[g73c]; i++)
        a72b2[i] = g99d6 + ((unsigned far *) g99d6)[i] + 2;
    for (; i < 0x28; i++)
        a72b2[i] = a72b2[0];
}


/* ---- F_462E (original code at 0x462E) ---- */
/* F_462E -- paint the title/menu backdrop for the current mode.  si is the
   mode, di the row offset chosen by the two-term disjunction at 4698. */
extern unsigned char g4374[];
extern char g0b3ae[];
extern int menu_resources_load(), face7();
extern void setmem();
extern void board_terrain_resources_load();

#include "R3E8.H"
extern struct record3e8 g43b4[];

void menu_backdrop_paint(void)
{
    register int m, d;

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
        setmem(g0b3ae, 0xbb8, 0);
    } else {
        if (face7() == 0 || m > 0x14)
            d = 0;
        else
            d = 0x330c;
        movmem(ui_gfx_shadow_a + d + 2, g4374, 0x2750);
        movmem(ui_gfx_shadow_a + d + 0x2754, g0b3ae, 0xbb8);
    }
    if (m < 0x14)
        board_terrain_resources_load(g4374[0] & 0x7f);
    board_records = (char far *) &g43b4[board_record_index = 0];
}


/* ---- F_4713 (original code at 0x4713) ---- */
/* F_4713 -- the level driver: set the map geometry, build the view, then run
   the turn loop in F_3A75 and hand back its result.  No frame at all (no
   parameters, no locals, one register variable), which is what -k- gives. */
extern void menu_backdrop_paint(void);
extern void puzzle_clear_grid(void);
extern void board_redraw_paint(void);
extern void roundend_round_setup(int n);
extern void menu_list_source_set_default(void), hud_scroll_reset(void);
extern void board_actors_draw();extern void gfx_clear_rect();extern void gfx_box();
extern void hud_panel_open();
extern void gfx_color_select(int n);
extern void anim_step_loop(int, int, int, int, int, int);
extern int tick_div8(), campaign_node_index(), turn_loop_run(), level_play_chapter();
extern void resource_record_cache_reset(int n);
extern void tutorial_hint_dialog_show(int);

extern unsigned char b4374, b4375, b4376;
extern int snd_flag2, g8bea, g8bec, g8bee, g8bf4, g8bf6, g8bf8;

int level_driver_run()
{
    register int r;

    menu_backdrop_paint();
    menu_list_source_set_default();
    cursor_x = b4375;
    cursor_x <<= 1;
    cursor_x = ((cursor_x + 3) >> 2) << 2;
    cursor_y = b4376;
    if (b4374 & 0x80) g73a = 1;
    else g73a = 0;
    g72c = gb07a = g72e = 0;
    if (value_parity(campaign_round_node_cursor)) {
        g722 = 3;
        g8bea = g8bec = g8bee = 0xf4;
        g8bf4 = 0x12;
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
/* F_48BE -- patch the blitter at IP 0x039C in place with the variant record
   for the current display mode.  The destination is a far pointer to CODE,
   widened from the near function with a cast, which is why it pushes
   `cs` and not a segment fixup. */
extern void far *memmove();
extern void runtime_base();
extern char display_mode;                      /* DS:BFCD, the display mode */

void blitter_patch_variant()
{
    register int n;

    if (display_mode == 5) {
        n = resource_load_record(2);
        memmove((char far *) runtime_base, ui_gfx_shadow_a, n);
    } else if (display_mode == 2) {
        n = resource_load_record(3);
        memmove((char far *) runtime_base, ui_gfx_shadow_a, n);
    }
}


/* ---- F_490D (original code at 0x490D) ---- */
/* F_490D -- boot init.  Entry 14A0D, 54 bytes.  A straight-line sequence
   of twelve calls, no branches.  Disassembly (assets/AEPROG.EXE):
       33C0 50 50 33C0 50   xor ax,ax; push ax; push ax; xor ax,ax; push ax
       E8 F8B0              call biostime          (1FB0F)   -- biostime(0, 0L):
                                                  the long/far arg is rightmost
                                                  (pushed first, one xor then
                                                  two pushes of the same zero
                                                  for its two halves), the int
                                                  arg is leftmost (pushed
                                                  second, its own fresh xor)
       83C406               add sp,6            (cdecl cleanup, 3 words)
       50                   push ax             (biostime's own return value)
       E8 E0AF              call srand          (1F9FE)   -- 1 arg
       59                   pop cx              (cdecl cleanup, 1 word --
                                                  TC 2.0 uses "pop reg" rather
                                                  than "add sp,2" to discard
                                                  exactly one pushed word)
       E8 5822              call timer_irq_install          (16C7A)   -- 0 args
       E8 3819              call dos_critical_error_install          (1635D)
       E8 1C8A              call ui_gfx_alloc          (1D444)
       E8 93FF              call blitter_patch_variant          (149BE)
       E8 53B9              call video_alloc_framebuffer          (10381)
       E8 248C              call player_record_load_publish          (1D655)
       E8 2A20              call keyboard_irq_install          (16A5E)
       FB                   sti                 (enable())
       E8 A3D8              call resource_stripe_table_load          (122DB)   -- 0 args
       E8 6ED8              call sprite_load_boot_sheets          (122A9)
       33C0 50              xor ax,ax; push ax
       E8 6523              call sprite_sheet_select          (16DA6)   -- 1 arg, 0
       59                   pop cx              (cdecl cleanup, 1 word)
       C3                   ret
   The two 0-arg calls immediately after srand's cleanup (timer_irq_install, dos_critical_error_install, ...)
   have no push before them and no pop/add after: void, no return value
   used. */
extern int  biostime();
extern void srand();
extern void timer_irq_install();
extern void dos_critical_error_install();
extern void ui_gfx_alloc();
extern void blitter_patch_variant();
extern void video_alloc_framebuffer();
extern void player_record_load_publish();
extern void keyboard_irq_install();
extern void resource_stripe_table_load();
extern void sprite_load_boot_sheets();
extern void sprite_sheet_select();

void boot_init_seed_rand()
{
    srand(biostime(0, 0L));
    timer_irq_install();
    dos_critical_error_install();
    ui_gfx_alloc();
    blitter_patch_variant();
    video_alloc_framebuffer();
    player_record_load_publish();
    keyboard_irq_install();
    asm sti;
    resource_stripe_table_load();
    sprite_load_boot_sheets();
    sprite_sheet_select(0);
}


/* ---- F_4943 (original code at 0x4943) ---- */
extern int level_driver_run();
#include "C470.H"
void campaign_chapter_advance(i) register int i;{slot_table[current_slot].resume_round=i+1;campaign_round_node_cursor=slot_table[current_slot].round_progress[i]*2+i*8;g73e=-1;while(1){if(value_parity(campaign_round_node_cursor)){if(!level_driver_run())campaign_round_node_cursor-=2;else{slot_table[current_slot].round_progress[i]++;if((campaign_round_node_cursor&7)==7)break;}}else{while(!level_driver_run());}campaign_round_node_cursor++;}}

/* ---- F_49E3 (original code at 0x49E3) ---- */
extern void sound_stop_reset(), sound_voices_reset(), keyboard_irq_restore(), timer_irq_restore();
void game_shutdown(void) { sound_stop_reset(); sound_voices_reset(); keyboard_irq_restore(); timer_irq_restore(); }


/* ---- F_49F0 (original code at 0x49F0) ---- */
/* F_49F0 -- "THE GAME": the top-level sequencer that calls the intro driver
   intro_run_chapter (157C6) and then runs the per-level record sequence.  Entry 14AF0,
   163 bytes, 16 activations per cold boot.

   Disassembly (assets/AEPROG.EXE, ndisasm -b16 -o 0x49f0):
       55 8BEC 83EC02 56 57   push bp; mov bp,sp; sub sp,2; push si; push di
       E8 CB0C                call intro_run_chapter
       3D FFFF                cmp ax,-1        (accumulator short form)
       75 03 / E9 8A00        jnz +3 / jmp 4A8D  -- "if (...) return;"
       E8 46D8                call hud_arena_init        (0 args, result unused)
       1E B8FE8B 50           push ds; mov ax,0x8BFE; push ax  -- (char far *)game_abort_jmpbuf
       E8 45AF                call setjmp
       59 59                  pop cx; pop cx    (cdecl cleanup, 4 bytes = 2 pops)
       8BF8                   mov di,ax         -- d = setjmp(game_abort_jmpbuf)
       BEFFFF                 mov si,0xFFFF     -- s = -1
       E8 E327 / E8 988B / E8 3B48   ui_overlay_reset(); sound_request_count_clear(); puzzle_free_resources();
       83FF03 7502 EB68       cmp di,3; jnz +2; jmp 4A8D   -- if (d == 3) return;
       83FF02 7D4D            cmp di,2; jnl 4A77           -- if (d < 2) { ... }
       E8 3961 3DFFFF 7502 EB59   if (slot_menu_run() == -1) return;
       A1ED13 BA1B00 F7E2     mov ax,[13ED]; mov dx,0x1B; mul dx
       8BD8 81C370C4          mov bx,ax; add bx,0xC470
       1E 07                  push ds; pop es          -- far-pointer form:
                                                          (char far *)slot_table + i*0x1B
       268A4715 98 50         mov al,[es:bx+0x15]; cbw; push ax
       E8 F628 59             call energy_set; pop cx
       ... the same address recomputed, field +0x0C this time ...
       268A470C 98 8946FE     mov al,[es:bx+0xC]; cbw; mov [bp-2],ax
       0BC0 740D              or ax,ax; jz 4A77        -- if (n = ...) {
       FF4EFE                 dec word [bp-2]          --   n--;
       8B76FE 8BC6 50         mov si,[bp-2]; mov ax,si; push ax
                                                       --   campaign_chapter_advance(s = n)
                                  (the assignment's VALUE is what is pushed:
                                   "s = n" loads si straight from memory, then
                                   the expression result goes through ax.  Two
                                   separate statements would have pushed si --
                                   see the loop body below, which does.)
       E8 CDFE 59             call campaign_chapter_advance; pop cx
       EB0C                   jmp short 4A85           -- while (s != 4) {
       56 E8 EF87 59          push si; call player_select_run; pop cx
       8BF0                   mov si,ax                --   s = player_select_run(s);
       56 E8 BFFE 59          push si; call campaign_chapter_advance; pop cx   -- campaign_chapter_advance(s);
       83FE04 75EF            cmp si,4; jnz 4A79       -- }
       E8 9862                call slot_archive_and_delete
       5F 5E 8BE5 5D C3       pop di; pop si; mov sp,bp; pop bp; ret

   The 27-byte stride at 0xC470 and the byte fields at +0x15 and +0x0C are
   the per-level record indexed by current_slot; the multiply is `mul` (unsigned),
   which is what TC 2.0 emits for `int * const` -- the same shape the already
   matched F_5AC3 carries at 5B35 (`mov dx,0x18; mul dx; mov bx,ax;
   add bx,0x900; push ds; pop es`). */

extern int  intro_run_chapter(), setjmp(), slot_menu_run(), player_select_run();
extern void hud_arena_init(), ui_overlay_reset(), sound_request_count_clear(), campaign_chapter_advance(), slot_archive_and_delete();
extern void puzzle_free_resources(void);
extern void energy_set(int state);

#include "C470.H"

extern char game_abort_jmpbuf[];                    /* DS:8BFE */

void game_run()
{
    int n;                              /* bp-2 */
    register int s, d;                  /* si, di */

    if (intro_run_chapter() == -1) return;
    hud_arena_init();
    d = setjmp(game_abort_jmpbuf);
    s = -1;
    ui_overlay_reset();
    sound_request_count_clear();
    puzzle_free_resources();
    if (d == 3) return;
    if (d < 2) {
        if (slot_menu_run() == -1) return;
        energy_set(slot_table[current_slot].state);
        if (n = slot_table[current_slot].resume_round) {
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


/* ---- F_4A93 (original code at 0x4A93) ---- */
/* F_4A93 -- main().  Entry 14B93, 20 bytes.  A straight-line sequence of
   five calls: video_mode_select() gates whether the rest ever runs (init failure ->
   immediate return with no side effects on the four other calls), then
   boot_init_seed_rand (boot init), game_run (the game), game_shutdown (shutdown), video_set_text_mode (exit).
   Disassembly (assets/AEPROG.EXE, physical 14B93):
       E8 74 07        call video_mode_select          (1530A)
       0B C0            or  ax,ax
       74 0C            jz  14BA6           (the ret at the end)
       E8 70 FE        call boot_init_seed_rand          (14A0D)
       E8 50 FF        call game_run          (14AF0)
       E8 40 FF        call game_shutdown          (14AE3)
       E8 A9 B8        call video_set_text_mode          (1044F)
       C3               ret
   No MOV sets ax before the single ret, so no "return <value>" appears in
   the source.  An "if (x == 0) return;" early-return shape does NOT
   reproduce these bytes: TC 2.0's unoptimized codegen for that shape is the
   "jnz +2 / jmp +N" idiom (two branches, 4 bytes) rather than a single
   inverted jcc, which overflows this 20-byte extent by 2 bytes (confirmed
   by a first compile attempt, T00.OBJ, kept as the negative test). The
   ORIGINAL shape is a single guarded block with no early exit: */
extern int  video_mode_select();
extern void boot_init_seed_rand();
extern void game_run();
extern void game_shutdown();
extern void video_set_text_mode();

main()
{
    if (video_mode_select()) {
        boot_init_seed_rand();
        game_run();
        game_shutdown();
        video_set_text_mode();
    }
}
