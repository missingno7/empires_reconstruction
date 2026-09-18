/* F_71FB -- clear the counter at DS:0B85.  No frame (rule 11).  The pilot
   draft spelled this global `_g_flag`, a hand-chosen name the declared
   naming convention cannot decide; it is spelled by the convention here. */
extern int gb85;                        /* DS:0B85 */

void f71fb()
{
    gb85 = 0;
}
