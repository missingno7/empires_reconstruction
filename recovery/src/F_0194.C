/* F_0194 -- clear the word at DS:0081.  The extent ends in `retf`, and the
   profile builds compact (`-mc`: far DATA, NEAR code), so the function itself
   is declared `far`.  No frame: no parameter and no local (rule 11). */
extern unsigned _8087;                  /* C0C public __8087, DS:0081 */

void far f0194()
{
    _8087 = 0;
}
