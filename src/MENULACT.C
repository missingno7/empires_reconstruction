/* F_792C -- read back the word at DS:0BB2.  EB00 as in F_6B74. */
extern int gbb2;                        /* DS:0BB2 */

int menu_list_active()
{
    return (gbb2);
}
