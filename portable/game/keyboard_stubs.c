/* keyboard_stubs.c -- stand-ins for not-yet-ported subsystems that
 * portable/game/keyboard.c calls into.  Each stub is replaced when its
 * owning module lands:
 *   menu_list_active / menu_loop_run -- the menu subsystem's F1..F10
 *     hotkey dispatch (src/KEYBOARD.C's F_6B1A callees).
 *
 * sound_effects_toggle (src/SNDFXTGL.C's port) used to be stubbed here;
 * portable/game/sndfxtgl.c now supplies the real definition, so it was
 * removed from this file to avoid a duplicate-definition link error.
 */
#include "dos_types.h"

dos_int menu_list_active(void)
{
    return 0;
}

void menu_loop_run(dos_int index)
{
    (void)index;
}
