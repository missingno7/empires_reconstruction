/*@FLAGS -B */
/* F_3A75 -- the turn loop.  Entry 13B75; the declared /24 player-query point
   13C08 is the rect_table_hit_id() probe at offset 0x93 of this function. */
struct C470 { char pad[0x15]; char b15; char rest[5]; };

extern int f6b4a(), f6b1a(), hud_tab_next(), rect_table_hit_id();
extern void timer_deadline_arm();
extern void player_select_restart_confirm();
extern void keyboard_chain_disable(void);
extern int puzzle_deal_pieces();extern void gfx_wipe_rect();
extern void record_table_delete_compact();
extern void fcaf1();
extern void f1ecd(void);
extern void board_scroll_transition();
extern void sprite_draw_cursor(void);
extern int energy_adjust(), f60a9(), anim_frame_advance();
extern void board_run_unit_script();
extern void board_unit_script_trigger(void);
extern void board_scan_wipe_effect(int);
extern void board_advance_unit_moves(int a);
extern int f_d79c(), fd386(), f1f91();extern void f4e9f();
extern void board_redraw_view();
extern void sprite_slots_redraw();
extern void board_update_moving_records();
extern int shadow_bitmap_hit_test(), hud_tab_get(), hud_scroll_move();extern void f4b0c();
extern void cursor_trail_arm(void);
extern void f329f(void);
extern void board_raycast_step(void);
extern void roundend_flash_panel_icons(void);
extern void timer_deadline_wait(void);
extern void f3986();
extern char far *record_field_skip_n();

extern int g0bc, g72c, g72e, g730, g732, g734, g736, g738, g73a;
extern int g8fe, gb68, gb6a, gb6c, gb6e, gb70, g71e, g722, current_slot;
extern int g40ce, g96e4, gb07a, board_record_index;
extern char far *rect_queue_write_ptr;                 /* the edge cursor */
#include "LAYOUT.H"
#include "VIDEO.H"
extern char far *ui_gfx_blob;
extern char far *board_records;                 /* vram */
extern char far *record_table_root;
extern unsigned char far *gbfc4;
extern char far *g40d0;                 /* objtab */
extern char far *resource_stripe_table;                 /* sprite base */
extern char str96ee[];
extern unsigned char b740[];
extern unsigned char b4380[], b4386[];
extern unsigned char b437a[];
extern char b438c[], b4396[], b43a0[], b43aa[];
extern int w8bea[], w8bf4[];
extern struct C470 c470[];

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
    g0bc = gb70 = 1;
    keyboard_chain_disable();
    for (;;) {
        timer_deadline_arm(0x18);
        g40ce = dir = key = 0;
        if (f6b4a()) {
            g0bc = 0;
            key = f6b1a();
            if (key == 0xd) hud_tab_next();
            else if (key == 0x1b) player_select_restart_confirm();
            g0bc = 1;
        }
        rect_queue_write_ptr = ui_gfx_blob;
        if ((obj = rect_table_hit_id((g736 >> 1) + 1, g738 + 1, 14, 0x27)) != 0 && obj != lastobj) {
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
                fcaf1(2);
                if (gb07a != 0) {
                    sprite_draw_cursor();
                    f1ecd();
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
                fcaf1(3);
            } else if (obj < 0x20) {
                p = record_field_skip_n(obj - 8);
                if (p[1] != 2) board_run_unit_script(p);
            } else if (obj < 0x30) {
                board_advance_unit_moves(obj - 0x20);
            }
        }
        lastobj = obj;
        if (*g40d0 != 0) f60a9();
        if (g8fe != 0) board_unit_script_trigger();
        if (gb68 != 0 && g722 != 0) {
            for (obj = 0; obj < g722; obj++) {
                if (w8bf4[obj] - 2 <= g738 && w8bf4[obj] + 2 >= g738 &&
                    w8bea[obj] <= g736 && w8bea[obj] + 0x10 >= g736) {
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
        f4e9f();
        if (g8fe != 0) sprite_slots_redraw();
        board_update_moving_records();
        if (*record_table_root != 0) f_d79c();
        if (*gbfc4 != 0) fd386();
        if (gb6e != 0) {
            if (x == 0 || gb68 == 0) {
                x = 0;
                g73a = x;
                if ((f1f91(g736 + 0x21, g738 + 1, 0x27) & 7) == 0) {
                    g736 += g734;
                    dir = 1;
                    if (g72e <= 8) {
                        if (++g72e > 8) g72e = 1;
                    }
                }
            }
        } else if (gb6c != 0) {
            if (x == 0 || gb68 == 0) {
                g73a = 1;
                x = 0;
                if ((f1f91(g736, g738 + 1, 0x27) & 7) == 0) {
                    g736 -= g734;
                    dir = -1;
                    if (g72e <= 8) {
                        if (++g72e > 8) g72e = 1;
                    }
                }
            }
        }
        if (gb68 != 0 && (f1f91(g736 + 0x10 - (g73a << 2), g738 + 1, 0x27) & 0x80) != 0) {
                if (x == 0) {
                    if (g736 % 8 == 0) {
                        if ((f1f91(g736 + 0x14, g738 + 1, 0x1d) & 0x80) != 0)
                            g736 += 4;
                        else
                            g736 -= 4;
                    }
                    x = 1;
                } else if ((shadow_bitmap_hit_test(g736 + 0xf, g738 - 4, 2) & 0x80) != 0) {
                    g738 -= 4;
                    if (++x > 2) x = 1;
                } else if ((shadow_bitmap_hit_test(g736 + 0xf, g738 - 2, 2) & 0x80) != 0) {
                    g738 -= 2;
                    if (++x > 2) x = 1;
                }
                g734 = 8;
                g730 = 0;
                g72e = x + 0x13;
        } else if (gb6a != 0 && x != 0) {
            if ((shadow_bitmap_hit_test(g736 + 0xf, g738 + 0x26, 2) & 0x80) != 0) {
                g738 += 4;
                if (++x > 2) x = 1;
                g72e = x + 0x13;
            } else {
                x = 0;
            }
        }
        if (x == 0) {
            if (g730 != 0) {
                if ((shadow_bitmap_hit_test(g736 + 8, g738 - 1, 9) & 7) == 0) {
                    g738 -= b740[g730];
                    g730--;
                    if (++g72e > 0xb) g72e = 0xb;
                    if (dir == 0) g72e = 0xa;
                } else {
                    g730 = 0;
                }
                goto moved;
            }
            if ((shadow_bitmap_hit_test(g736 + 8, g738 + 0x2f, 9) & 7) == 0) {
                if (g732 != 0) {
                    g738 += 8;
                } else {
                    g732 = 1;
                    g738 += 2;
                }
                g72e = (dir & 1) + 0xa;
                goto moved;
            }
            if (((probe = shadow_bitmap_hit_test(g736 + 8, g738 + 0x28, 9)) & 7) == 0) {
                g738 += (((g738 + 0x30) / 8) << 3) - (g738 + 0x28);
                g732 = 0;
                g72e = (dir & 1) + 0xa;
                fcaf1(0xb);
                goto moved;
            }
            if (key == 0x20) {
                if (hud_tab_get() == 1) {
                    if ((shadow_bitmap_hit_test(g736 + 8, g738 - 1, 9) & 7) == 0) {
                        fcaf1(0x10);
                        g730 = 8;
                        g72e = 9;
                        g734 = dir ? 8 : 4;
                    } else goto fell;
                } else goto fell;
                goto moved;
            }
            if (gb68 != 0 && gb70 != 0) {
                if ((shadow_bitmap_hit_test(g736 + 8, g738 - 1, 9) & 7) == 0) {
                    gb70 = 0;
                    g730 = 5;
                    g72e = 9;
                    g734 = dir ? 8 : 4;
                    fcaf1(0xc);
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
                    if ((f1f91(g736, g738 + 1, 0x27) & 7) == 0) {
                        g736 -= g734;
                        if (g736 <= -4) g736 = -0x11;
                    }
                } else {
                    if ((f1f91(g736 + 0x21, g738 + 1, 0x27) & 7) == 0) {
                        g736 += g734;
                        if (g736 >= 0x130) g736 = 0x131;
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
                        fcaf1(0);
                    } else {
                        fcaf1(0x11);
                    }
            } else if (obj == 0 && x == 0) {
                if (g8fe == 0) {
                    cursor_trail_arm();
                    fcaf1(0x14);
                } else {
                    fcaf1(0x17);
                }
            }
        }
        if (g72c == 1) g72c = 0;
        f4b0c();
        if (g738 < 0) {
            if (b43a0[board_record_index] != 0) {
                g738 = 0x90;
                if (b43a0[board_record_index] - 1 != board_record_index) {
                    board_record_index = b43a0[board_record_index] - 1;
                    g8fe = lastcur = lastobj = 0;
                    f329f();
                }
            } else {
                g738 = 0;
            }
        } else if (g738 > 0x90) {
            if (b43aa[board_record_index] != 0) {
                g738 = 0;
                if (b43aa[board_record_index] - 1 != board_record_index) {
                    board_record_index = b43aa[board_record_index] - 1;
                    g8fe = lastcur = lastobj = 0;
                    f329f();
                }
            } else {
                g738 = 0x90;
            }
        }
        if (g736 < -0x10) {
            if (b438c[board_record_index] != 0) {
                g736 = 0x120;
                if (b438c[board_record_index] - 1 != board_record_index) {
                    board_record_index = b438c[board_record_index] - 1;
                    g8fe = lastcur = lastobj = 0;
                    f329f();
                }
            } else {
                g736 = -0x10;
            }
        } else if (g736 > 0x130) {
            if (b4396[board_record_index] != 0) {
                g736 = 0;
                if (b4396[board_record_index] - 1 != board_record_index) {
                    board_record_index = b4396[board_record_index] - 1;
                    g8fe = lastcur = lastobj = 0;
                    f329f();
                }
            } else {
                g736 = 0x130;
            }
        }
        if (g8fe != 0) board_raycast_step();
        if (blink != 0) {
            blink--;
            if (blink > 0x1a) {
                gfx_copy_rect(g736, g738, resource_stripe_table + 0x39ec, g73a);
            } else if (blink & 1) {
                gfx_copy_rect(g736, g738, resource_stripe_table + g72e * 0x2a2, g73a);
            }
        } else if (g72c != 0) {
            gfx_copy_rect(g736, g738, (char far *)str96ee, 0);
            g72c--;
            gfx_copy_rect(g736, g738, resource_stripe_table + g72e * 0x2a2, g73a);
        } else {
            if (g40ce != 0 && lastcur != g40ce) {
            gfx_copy_rect(g736, g738, resource_stripe_table + 0x39ec, g73a);
            blink = 0x1e;
            fcaf1(1);
            f1ecd();
            g0bc = 0;
            if (energy_adjust(-1) == 0) {
                f3986();
                return 0;
            }
            c470[current_slot].b15 = energy_adjust(0);
            g0bc = 1;
            x = 0;
            lastcur = g40ce;
            goto tail;
            } else {
            gfx_copy_rect(g736, g738, resource_stripe_table + g72e * 0x2a2, g73a);
            }
            lastcur = g40ce;
        }
        f1ecd();
tail:
        timer_deadline_wait();
        if (g71e != 0 && g96e4 == 0 && g736 > 0xbe) {
            roundend_flash_panel_icons();
            g96e4 = 1;
        }
    }
    g0bc = 0;
    return 1;
}
