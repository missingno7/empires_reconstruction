/* src/ROUNDEND.C: Round-end sequence. */
#include "game.h"

struct HDR { dos_char pad[0x122]; dos_int f122; };

/* PORT: a1271 (src/ROUNDEND.C: `extern struct { int w0; int w2; } a1271[];`)
   is generated as a flat `uint8_t a1271[48]` (portable/generated/game_data.h)
   instead of a 12-element (int,int) array -- still open as of this write;
   the byte layout is identical (4-byte little-endian pairs), so this adapts
   the indexing rather than stopping, using dos_rd16 (portable/include/
   dos_types.h) to read each 16-bit field out of the flat blob. */
#define A1271_W0(y) ((dos_int)dos_rd16(&a1271[(y) * 4]))
#define A1271_W2(y) ((dos_int)dos_rd16(&a1271[(y) * 4 + 2]))

/* ---- F_9A0E (original code at 0x9A0E) ---- */
/* F_9A0E -- run one entry of the 18h-stride script table g0DCC: notify
   f99E2/f984C, and either draw the "selected" bitmap out of the far blob
   gC5C6 at the offset its own header word 122h names (when the entry is the
   current one and gC354 is clear), or draw the bitmap the entry's byte names
   and dispatch through the handler table g12A1 by its signed byte. */

void roundend_marker_show(dos_int n, dos_int j)
{
    score_set_position(n);
    score_panel_clear();
    if (n == gc359 && gc354 == 0) {
        gfx_color_select(5);
        gfx_blit_image(score_panel_x, score_panel_y, ui_gfx_shadow_a + ((struct HDR *) ui_gfx_shadow_a)->f122 + 2);
        gfx_color_select(0);
    } else {
        /* PORT: g0dcc is now `struct g0dcc_entry g0dcc[40]`
           (portable/generated/game_data.h) and g12a1 is now a real
           `void (*g12a1[6])(void)` (supervisor decision, generator fix
           landed) -- straight struct-field/function-pointer-table access,
           no adaptation needed any more. */
        gfx_blit_image(0, 0x168,
              ui_gfx_shadow_a + ((dos_int *) ui_gfx_shadow_a)[g0dcc[gc35d].e[j].idx] + 2);
        (*g12a1[g0dcc[gc35d].e[j].sel])();
    }
}


/* ---- F_9AC7 (original code at 0x9AC7) ---- */
void roundend_flash_panel_icons(void)
{
    dos_int i;

    for (i = 0; i < 3; i++) {
        stream_control_block_arm(0x15);
        gfx_wipe_rect(0xf4, i * 0x30 + 0x158, 0x30, 0x23, 0xf4, i * 0x30 + 0x10);
        gfx_box(0xf4, i * 0x30 + 0x10, 0x30, 0x23);
        gfx_wipe_rect(0xf4, i * 0x30 + 0x158, 0x30, 0x23, 0xf4, i * 0x30 + 0xc8);
        timer_wait_ticks(0x78);
        sound_stop_reset();
    }
    tutorial_hint_dialog_show(5);
}


/* ---- F_9B68 (original code at 0x9B68) ---- */
/* F_9B68 -- the round opener: clear the three panels, roll a bit out of the
   scenario record, draw the 12 markers, then reset the panels. */

void roundend_round_setup(dos_int n)
{
    dos_int a6;
    dos_int a4;
    dos_int i;
    dos_int x, y;

    x = n;
    for (i = 0; i < 3; i++)
        gfx_wipe_rect(0xf4, i * 0x30 + 0xc8, 0x30, 0x23, 0xf4, i * 0x30 + 0x10);
    gc35b = gc354 = gc35b = 0;
    gc35d = slot_is_new_game() * 0x14 + x;
    /* PORT: rand() -- Turbo C's LCG differs from any modern libc's; see
       docs/portable/int-semantics-inventory.md sec "RNG-touching clusters".
       Not resolved here (out of this agent's scope); reported as-is. */
    while ((g0dcc[gc35d].f16 & (1 << (x = rand() % 8))) == 0)
        ;
    gc359 = x + 1;
    resource_load_record(0x1022);
    for (y = 0; y < 0xc; y++) {
        a6 = A1271_W0(y);
        a4 = A1271_W2(y);
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
void roundend_wait(void) { sound_stop_reset(); stream_control_block_arm(9); while (snd_on) ; }
