/* sndreq.c -- portable port of src/SNDREQ.C: sound request start and
 * request counter.
 * One translation unit; the sections below were the separate member
 * sources of grouped module C_D593_D5B3 and keep their original ids.
 */
#include "game.h"

/* ---- F_D593 (original code at 0xD593) ---- */
void sound_start(void)
{
    sound_stop_reset();
    sound_request_count++;
    timer_wait_ticks(1);
    sound_voices_disable_all();
}

/* ---- F_D5A6 (original code at 0xD5A6) ---- */
void sound_request_count_dec(void)
{
    if (--sound_request_count < 0)
        sound_request_count = 0;
}

/* ---- F_D5B3 (original code at 0xD5B3) ---- */
void sound_request_count_clear(void)
{
    sound_request_count = 0;
}
