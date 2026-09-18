/* F_7925 -- clear the word at DS:0BB2.  No frame (rule 11). */
extern int gbb2;                        /* DS:0BB2 */

void f7925()
{
    gbb2 = 0;
}
