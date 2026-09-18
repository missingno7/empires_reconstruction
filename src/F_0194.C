/* F_0194 -- clear the word at DS:0081.  The extent ends in `retf`, and the
   profile builds compact (`-mc`: far DATA, NEAR code), so the function itself
   is declared `far`.  No frame: no parameter and no local (rule 11). */
extern unsigned g81;                    /* DS:0081 */

void far f0194()
{
    g81 = 0;
}
