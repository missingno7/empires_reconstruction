/* F_6990 -- set the flag at DS:0B72.  No frame (rule 11). */
extern int gb72;                        /* DS:0B72 */

void f6990()
{
    gb72 = 1;
}
