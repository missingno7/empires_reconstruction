/* timer.c -- historical 236.7 Hz tick service (src/TIMER.C), platform-agnostic
 * half.  See docs/portable/architecture.md "Timing model" and timer.h.
 *
 * timer_service_tick() is the body of the historical INT 8 handler
 * (src/TIMER.C F_6BCF) minus the DOS/PIC plumbing it was wrapped in
 * (PUSHF/STI, the 13:1 chain to the saved BIOS vector, OUT 0x20,0x20 EOI):
 * those are DOS interrupt mechanics, not game state, and are dropped per
 * timer.h's "Dropped" note.
 *
 * Manual mode ("no platform thread; time advances only when someone waits
 * or steps"): timer_platform_wait_tick() returns immediately in manual mode
 * (see clock.c / test stand-ins), so a wait loop that only blocked on it
 * would spin forever without timer_ticks ever advancing.  timer_wait_ticks()
 * and timer_deadline_wait() therefore call timer_service_tick() themselves,
 * once per loop iteration, whenever manual mode is set -- that is what
 * actually advances the clock during a manual-mode wait.
 */
#include "timer.h"

#include <stdbool.h>

/* DGROUP state this module owns (declared extern in timer.h). */
volatile dos_ulong timer_ticks;  /* DS:0B76 free-running tick counter */
dos_ulong gc0d0;                 /* DS:C0D0 armed deadline */

/* Sound engine hooks (src/TIMER.C F_6BCF).  The real driver lives in
 * portable/audio (not written yet as of Phase 7); portable/audio/sound_stub.c
 * defines these four so timer.c links today -- see that file's header
 * comment for the removal plan. */
extern dos_int sound_request_count, sound_enabled, music_enabled;
extern void sound_tick_entry(void);

static bool s_manual = false;

void timer_service_tick(void)
{
    ++timer_ticks;
    if (!sound_request_count && (sound_enabled || music_enabled))
        sound_tick_entry();
}

/* F_6C26: t = n + timer_ticks.  n is a signed 16-bit int (dos_int); the
 * historical codegen sign-extends it with `cwd` before the 32-bit add
 * (tc20-codegen rule 5), so the cast chain below goes dos_int -> dos_long
 * (sign-extend) -> dos_ulong (reinterpret as the unsigned operand of the
 * add).  The loop compares timer_ticks < t UNSIGNED, literally, including
 * across 0xFFFFFFFF wraparound -- see timer_deadline_reached() below for the
 * same literal-unsigned-compare rule applied to the deadline helpers. */
void timer_wait_ticks(dos_int n)
{
    dos_ulong t = (dos_ulong)(dos_long)n + timer_ticks;
    while (timer_ticks < t) {
        if (s_manual)
            timer_service_tick();
        else
            timer_platform_wait_tick();
    }
}

/* F_6C57: gc0d0 = n + timer_ticks, same sign-extension as timer_wait_ticks. */
void timer_deadline_arm(dos_int n)
{
    gc0d0 = (dos_ulong)(dos_long)n + timer_ticks;
}

/* F_6C6F: spin until the armed deadline passes (same manual-mode rule as
 * timer_wait_ticks -- see file header comment). */
void timer_deadline_wait(void)
{
    while (timer_ticks < gc0d0) {
        if (s_manual)
            timer_service_tick();
        else
            timer_platform_wait_tick();
    }
}

/* F_6C87: `if (timer_ticks < gc0d0) return 0; return 1;` -- both sides
 * compared UNSIGNED (ja/jb/jae on the raw 32-bit halves per tc20-codegen
 * rule 5).  Preserved literally: a deadline that has wrapped past
 * 0xFFFFFFFF while timer_ticks has not yet wrapped reads as "already
 * reached" even though real elapsed time has not caught up -- this is the
 * historical quirk, not a bug to fix here. */
dos_int timer_deadline_reached(void)
{
    if (timer_ticks < gc0d0)
        return 0;
    return 1;
}

/* F_6B7A / F_6BAC minus the DOS vector save/restore and PIT port I/O: those
 * become the platform tick thread's lifetime.  Manual mode never starts a
 * real thread -- tests step time by hand instead. */
void timer_irq_install(void)
{
    if (!s_manual)
        timer_platform_start();
}

void timer_irq_restore(void)
{
    if (!s_manual)
        timer_platform_stop();
}

void timer_service_set_manual(bool manual)
{
    s_manual = manual;
}

bool timer_service_is_manual(void)
{
    return s_manual;
}
