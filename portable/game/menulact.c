/* menulact.c -- portable port of src/MENULACT.C.
 *
 * F_792C -- read back the word at DS:0BB2.
 */
#include "game.h"

dos_int menu_list_active(void)
{
    return menu_list_enabled;
}
