/* F_7925 -- clear the word at DS:0BB2.  No frame (rule 11). */
extern int gbb2;                        /* DS:0BB2 */

void menu_list_disable()
{
    gbb2 = 0;
}
