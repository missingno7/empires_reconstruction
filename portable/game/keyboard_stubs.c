/* keyboard_stubs.c -- stand-ins for not-yet-ported subsystems that
 * portable/game/keyboard.c calls into.  Each stub is replaced when its
 * owning module lands:
 *   sound_effects_toggle -- SNDFXTGL.C's port (Ctrl+S sound toggle).
 *   menu_list_active / menu_loop_run -- the menu subsystem's F1..F10
 *     hotkey dispatch (src/KEYBOARD.C's F_6B1A callees).
 */
#include "dos_types.h"

void sound_effects_toggle(void)
{
    /* SNDFXTGL.C's port replaces this. */
}

dos_int menu_list_active(void)
{
    return 0;
}

void menu_loop_run(dos_int index)
{
    (void)index;
}
