/* menuldis.c -- portable port of src/MENULDIS.C.
 *
 * F_7925 -- clear the word at DS:0BB2.
 */
#include "game.h"

void menu_list_disable(void)
{
    menu_list_enabled = 0;
}
