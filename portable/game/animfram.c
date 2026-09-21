/* src/ANIMFRAM.C: advance the round-end animation one frame, or finish it. */
#include "game.h"

/* ---- F_9DCC (original code at 0x9DCC) ---- */
/* F_9DCC -- advance the animation one frame, or finish it.  si/di are the
   two register variables holding the blit origin; the char field stores are
   written as the assignment inside the call argument. */
dos_int anim_frame_advance(dos_int n)
{
    dos_int x, y;

    gc35b++;
    if (n + 9 != gc352) {
        roundend_wait();
        if (gc35b == 1) {
            /* g0dcc is `struct g0dcc_entry g0dcc[40]`
               (portable/generated/game_data.h); f17 is a signed
               dos_char (include/G0DCC.H), promoted here as written. */
            resource_load_record(g0dcc[gc35d].f17 + 0x1023);
            hud_prompt_message_run((dos_char *)(ui_gfx_blob + 2));
            return 0;
        } else {
            dialog_energy_lost_show();
            energy_set(slot_table[current_slot].state = 4);
            return -1;
        }
    }
    if (gc35b == 1) {
        energy_adjust(slot_table[current_slot].state = 4);
    }
    score_set_position(gc359);
    x = score_panel_x;
    y = score_panel_y;
    score_set_position(gc352);
    gc354 = 1;
    gfx_wipe_rect(score_panel_x, score_panel_y, 0x28, 0x1d, x, y - 0xb8);
    gfx_box(x, y - 0xb8, 0x28, 0x1d);
    return gc35b;
}
