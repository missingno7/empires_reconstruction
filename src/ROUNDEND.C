/* src/ROUNDEND.C: Round-end sequence.
   One translation unit; the sections below were the separate member
   sources of grouped module C_9A0E_9D79 and keep their original ids. */

struct HDR { char pad[0x122]; int f122; };
#include "G0DCC.H"
#include "VIDEO.H"

extern int gc359, gc354, gc35d, gc35b, gc352, score_panel_x, score_panel_y, g1770;
extern char far *ui_gfx_shadow_a;
extern void (*g12a1[])(void);
extern void score_set_position(int n);
extern void score_panel_clear(void);
extern void gfx_color_select(int n);
extern void gfx_blit_image(int x, int y, char far *p);
extern void stream_control_block_arm(int n);
extern void wipe(int x, int y, int w, int h, int x2, int y2);   /* 03B4 */
/*@SYM _wipe=0x03B4 kind=f key=functions/F_03B4.entry*/
extern void box(int x, int y, int w, int h);                    /* 039F */
/*@SYM _box=0x039F kind=f key=functions/F_039F.entry*/
extern void timer_wait_ticks(int n);
extern void sound_stop_reset(void);
extern void tutorial_hint_dialog_show(int n);
extern int face7(), rand(), resource_load_record();
extern void roundend_marker_show(int, int);
extern void rect_border_draw();
extern char gc356[];
extern struct { int w0; int w2; } a1271[];

/* ---- F_9A0E (original code at 0x9A0E) ---- */
/* F_9A0E -- run one entry of the 18h-stride script table g0DCC: notify
   f99E2/f984C, and either draw the "selected" bitmap out of the far blob
   gC5C6 at the offset its own header word 122h names (when the entry is the
   current one and gC354 is clear), or draw the bitmap the entry's byte names
   and dispatch through the handler table g12A1 by its signed byte. */

/*@PUB _roundend_marker_show*/
void roundend_marker_show(int n, int j)
{
    score_set_position(n);
    score_panel_clear();
    if (n == gc359 && gc354 == 0) {
        gfx_color_select(5);
        gfx_blit_image(score_panel_x, score_panel_y, ui_gfx_shadow_a + ((struct HDR far *) ui_gfx_shadow_a)->f122 + 2);
        gfx_color_select(0);
    } else {
        gfx_blit_image(0, 0x168,
              ui_gfx_shadow_a + ((int far *) ui_gfx_shadow_a)[g0dcc[gc35d].e[j].idx] + 2);
        (*g12a1[g0dcc[gc35d].e[j].sel])();
    }
}


/* ---- F_9AC7 (original code at 0x9AC7) ---- */
void roundend_flash_panel_icons(void)
{
    int i;

    for (i = 0; i < 3; i++) {
        stream_control_block_arm(0x15);
        wipe(0xf4, i * 0x30 + 0x158, 0x30, 0x23, 0xf4, i * 0x30 + 0x10);
        box(0xf4, i * 0x30 + 0x10, 0x30, 0x23);
        wipe(0xf4, i * 0x30 + 0x158, 0x30, 0x23, 0xf4, i * 0x30 + 0xc8);
        timer_wait_ticks(0x78);
        sound_stop_reset();
    }
    tutorial_hint_dialog_show(5);
}


/* ---- F_9B68 (original code at 0x9B68) ---- */
/* F_9B68 -- the round opener: clear the three panels, roll a bit out of the
   scenario record, draw the 12 markers, then reset the panels. */

void roundend_round_setup(int n)
{
    int a6;
    int a4;
    int i;
    register int x, y;

    x = n;
    for (i = 0; i < 3; i++)
        gfx_wipe_rect(0xf4, i * 0x30 + 0xc8, 0x30, 0x23, 0xf4, i * 0x30 + 0x10);
    gc35b = gc354 = gc35b = 0;
    gc35d = face7() * 0x14 + x;
    while ((g0dcc[gc35d].f16 & (1 << (x = rand() % 8))) == 0)
        ;
    gc359 = x + 1;
    resource_load_record(0x1022);
    for (y = 0; y < 0xc; y++) {
        a6 = a1271[y].w0;
        a4 = a1271[y].w2;
        gfx_color_select(6);
        rect_border_draw(a6 - 2, a4 - 2, 0x2e, 0x21);
        gfx_color_select(7);
        rect_border_draw(a6 - 1, a4 - 1, 0x2c, 0x1f);
        gfx_color_select(0xf);
        gfx_clear_rect(a6, a4, 0x2a, 0x1d);
    }
    for (x = 0; x < 9; x++)
        roundend_marker_show(x, x);
    gc356[0] = 9;
    gc356[1] = 0xa;
    gc356[2] = 0xb;
    x = rand() % 3;
    gc352 = x + 9;
    roundend_marker_show(gc352, gc359);
    gc356[x] = gc356[2];
    x = rand() % 2;
    roundend_marker_show(gc356[x], 9);
    roundend_marker_show(gc356[x ^ 1], 0xa);
    for (x = 0; x < 3; x++) {
        gfx_wipe_rect(0xf4, x * 0x30 + 0xc8, 0x30, 0x23, 0xf4, x * 0x30 + 0x158);
        gfx_wipe_rect(0xf4, x * 0x30 + 0x10, 0x30, 0x23, 0xf4, x * 0x30 + 0xc8);
    }
}


/* ---- F_9D79 (original code at 0x9D79) ---- */
void roundend_wait(void) { sound_stop_reset(); stream_control_block_arm(9); while (g1770) ; }
