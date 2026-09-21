/* F_D5B3 -- clear the counter at DS:237C.  No frame: no parameter and no
   local (rule 11). */
extern int g237c;                       /* DS:237C */

void sound_request_count_clear()
{
    g237c = 0;
}
