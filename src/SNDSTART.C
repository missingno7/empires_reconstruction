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
