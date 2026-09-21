/* menulsrc.c -- src/MENULSRC.C: point the active menu list at a caller's
 * list, or fall back to the default blank row (g0bb4) and disable the list.
 *
 * generator type issue: gc0fe (DS:C0FE) is historically `struct S far *`
 * (src/MENULIST.C:7 / GC0FE.H) but game_state.h emits it as a fallback
 * `uint8_t *gc0fe;` because "historical 'struct S' has no portable
 * definition yet".  The assignments below are plain pointer-value copies
 * (this file never dereferences gc0fe's pointee), so the fallback uint8_t*
 * type is semantically fine here; the cast is only to satisfy strict
 * pointer-type checking (dos_char* / uint8_t* both being byte pointers).
 */
#include "game.h"

/* F_?  -- point the menu list source at p, or reset to the blank default. */
void menu_list_source_set(dos_char *p)
{
    if (p) {
        gc0fe = (uint8_t *)p;
        menu_list_draw(-1);
        menu_list_enable();
    } else {
        gc0fe = (uint8_t *)g0bb4;
        menu_list_disable();
    }
}
