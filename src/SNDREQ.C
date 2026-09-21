/* src/SNDREQ.C: Sound request start and request counter.
   One translation unit; the sections below were the separate member
   sources of grouped module C_D593_D5B3 and keep their original ids. */

/* ---- F_D593 (original code at 0xD593) ---- */
/* F_D593 -- start one sound: reset the module at F_CB48, count the request
   at DS:237C, wait one tick and hand off to F_C877. */
extern void sound_stop_reset();
extern void timer_wait_ticks();
extern void sound_voices_disable_all();
extern int sound_request_count;                       /* DS:237C */

void sound_start()
{
    sound_stop_reset();
    sound_request_count++;
    timer_wait_ticks(1);
    sound_voices_disable_all();
}


/* ---- F_D5A6 (original code at 0xD5A6) ---- */
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


/* ---- F_D5B3 (original code at 0xD5B3) ---- */
/* F_D5B3 -- clear the counter at DS:237C.  No frame: no parameter and no
   local (rule 11). */
extern int sound_request_count;                       /* DS:237C */

void sound_request_count_clear()
{
    sound_request_count = 0;
}
