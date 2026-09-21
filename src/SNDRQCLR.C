/* F_D5B3 -- clear the counter at DS:237C.  No frame: no parameter and no
   local (rule 11). */
extern int sound_request_count;                       /* DS:237C */

void sound_request_count_clear()
{
    sound_request_count = 0;
}
