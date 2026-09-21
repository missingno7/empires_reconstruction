/* dlgelost.c -- portable port of src/DLGELOST.C.
 *
 * F_9D8E -- the "lost the pieces" dialog.  text118c/g125d are ported-C-owned
 * DATA (docs/portable/state-map.md "Ported-C-owned DATA": TEXT_118C at
 * DS:118C, DATA_125D at DS:125D, both code_owner F_9D8E) -- the generator
 * intentionally leaves these bytes to this translation unit's own static
 * initializers.  No other historical file references either name.
 */
#include "game.h"

/* TEXT_118C (DS:118C, 209 bytes). */
dos_char text118c[] =
    "Sorry, better luck next time!\rYou lost the pieces to the last\r"
    "artifact.\rYou must go back, find them again,\rand put them together.\r"
    "Then try again to break this\rchamber's code.\r\rYour Energy meter will be reset.";

/* DATA_125D (DS:125D, 20 bytes: struct dialog). */
struct dialog g125d = { 1, 0, 2, text118c, 0, -1, -1, -1, -1 };

void dialog_energy_lost_show(void)
{
    gfx_color_select(0);
    gfx_clear_rect(8, 16, 304, 145);
    gfx_box(8, 16, 304, 145);
    dialog_run(&g125d);
}
