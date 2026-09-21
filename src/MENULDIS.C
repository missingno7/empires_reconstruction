/* F_7925 -- clear the word at DS:0BB2.  No frame (rule 11). */
extern int menu_list_enabled;                        /* DS:0BB2 */

void menu_list_disable()
{
    menu_list_enabled = 0;
}
