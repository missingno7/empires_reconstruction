/* F_4713 -- the level driver: set the map geometry, build the view, then run
   the turn loop in F_3A75 and hand back its result.  No frame at all (no
   parameters, no locals, one register variable), which is what -k- gives. */
extern int value_parity();
extern void menu_backdrop_paint(void);
extern void puzzle_clear_grid(void);
extern void board_redraw_paint(void);
extern void roundend_round_setup(int n);
extern void f7df1(void), hud_scroll_reset(void);
extern void f4eeb();extern void f03a8();extern void f039f();
extern void hud_panel_open();
extern void gfx_color_select(int n);
extern void sprite_draw_cursor(void);
extern void anim_step_loop(int, int, int, int, int, int);
extern int tick_div8(), campaign_node_index(), turn_loop_run(), level_play_chapter();
extern void resource_record_cache_reset(int n);
extern void tutorial_hint_dialog_show(int);

extern unsigned char b4374, b4375, b4376;
extern int g71e, g722, g72c, g72e, g736, g738, g73a, g73e;
extern int g1776, g8bea, g8bec, g8bee, g8bf4, g8bf6, g8bf8, g9ade, gb07a;

int level_driver_run()
{
    register int r;

    menu_backdrop_paint();
    f7df1();
    g736 = b4375;
    g736 <<= 1;
    g736 = ((g736 + 3) >> 2) << 2;
    g738 = b4376;
    if (b4374 & 0x80) g73a = 1;
    else g73a = 0;
    g72c = gb07a = g72e = 0;
    if (value_parity(g9ade)) {
        g722 = 3;
        g8bea = g8bec = g8bee = 0xf4;
        g8bf4 = 0x12;
        g8bf6 = 0x42;
        g8bf8 = 0x72;
        g71e = 1;
        board_redraw_paint();
        roundend_round_setup(g9ade >> 1);
    } else {
        g722 = g71e = 0;
        puzzle_clear_grid();
        hud_scroll_reset();
        board_redraw_paint();
    }
    if (g73e == 0) {
        gfx_color_select(1);
        f03a8(8, 0x10, 0x130, 0x90);
    }
    anim_step_loop(8, 0xc8, 0x130, 0x90, 8, 0x10);
    f4eeb(0);
    sprite_draw_cursor();
    hud_panel_open();
    f039f(0, 0, 0x140, 0xc8);
    if (tick_div8() != 4 && (g9ade & 7) == 0) {
        tutorial_hint_dialog_show(0);
        tutorial_hint_dialog_show(1);
        if (tick_div8() == 1) tutorial_hint_dialog_show(2);
        else if (tick_div8() == 2) tutorial_hint_dialog_show(3);
    }
    g1776 = 1;
    if (value_parity(g9ade) == 0) {
        if (tick_div8() != 4)
            resource_record_cache_reset((tick_div8() << 2) + (g9ade & 2) + 0x1073);
        else
            resource_record_cache_reset((campaign_node_index() << 2) + 0x1073);
    }
    r = turn_loop_run();
    if (r != 0 && g9ade == 0x27) {
        r = level_play_chapter();
        g9ade = 0x27;
    }
    return r;
}
