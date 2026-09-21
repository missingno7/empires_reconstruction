/* F_9DCC -- advance the animation one frame, or finish it.  si/di are the
   two register variables holding the blit origin; the char field stores are
   written as the assignment inside the call argument. */
#include "G0DCC.H"
#include "C470.H"

extern int score_panel_x, score_panel_y, gc352, gc354, gc359, gc35b, gc35d;
#include "LAYOUT.H"
extern char far *ui_gfx_blob;
extern void roundend_wait(), f9d8e(), f039f();
extern int resource_load_record(), energy_adjust();extern void f03b4();
extern void f7747();
extern void score_set_position();
extern void energy_set(int state);

int anim_frame_advance(int n)
{
    register int x, y;

    gc35b++;
    if (n + 9 != gc352) {
        roundend_wait();
        if (gc35b == 1) {
            resource_load_record(g0dcc[gc35d].f17 + 0x1023);
            f7747(ui_gfx_blob + 2);
            return 0;
        } else {
            f9d8e();
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
    f03b4(score_panel_x, score_panel_y, 0x28, 0x1d, x, y - 0xb8);
    f039f(x, y - 0xb8, 0x28, 0x1d);
    return gc35b;
}
