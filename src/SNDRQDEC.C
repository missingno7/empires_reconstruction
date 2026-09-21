/* F_D5A6 -- count the counter at DS:237C down and clamp it at zero.  The
   `jnl` on the DECREMENT's own flags is what makes this ONE statement:
   `dec word [x]` sets SF, so no reload and no compare is emitted (rule 21).
   `jnl` is signed, so the counter is a signed int (rule 5). */
extern int sound_request_count;                       /* DS:237C */

void sound_request_count_dec()
{
    if (--sound_request_count < 0)
        sound_request_count = 0;
}
