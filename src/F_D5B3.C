/* F_D5B3 -- clear the counter at DS:237C.  No frame: no parameter and no
   local (rule 11). */
extern int g237c;                       /* DS:237C */

void fd5b3()
{
    g237c = 0;
}
