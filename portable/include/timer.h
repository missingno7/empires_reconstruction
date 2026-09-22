/* timer.h -- historical 236.7 Hz tick service (src/TIMER.C) over a portable clock.
 *
 * Historical facts preserved:
 *   PIT divisor 0x13B1 = 5041 -> 1193182 / 5041 = 236.6975 Hz tick rate.
 *   INT 8 body per tick: ++timer_ticks; if (!sound_request_count &&
 *   (sound_enabled || music_enabled)) sound_tick_entry();
 *   timer_wait_ticks / timer_deadline_* compare UNSIGNED 32-bit counters.
 * Dropped: the 13:1 chain to the BIOS vector, PIT/PIC port I/O.
 *
 * Threading: the platform runs timer_service_tick() from a dedicated thread
 * at the historical rate; the game thread blocks in the wait helpers.  Tests
 * may call timer_service_tick() by hand (manual stepping) after
 * timer_service_set_manual(true).
 */
#ifndef PORTABLE_TIMER_H
#define PORTABLE_TIMER_H

#include "dos_types.h"

#define TIMER_PIT_HZ        1193182.0
#define TIMER_PIT_DIVISOR   0x13B1
#define TIMER_TICK_HZ       (TIMER_PIT_HZ / TIMER_PIT_DIVISOR)

/* Historical DGROUP state (defined in portable/game/timer.c). */
extern volatile dos_ulong timer_ticks;   /* DS:0B76 free-running tick counter */
extern dos_ulong gc0d0;                  /* DS:C0D0 armed deadline */

/* The INT 8 body.  Called once per tick by the platform timer thread (or by
 * tests).  Runs sound_tick_entry() under the historical gate. */
void timer_service_tick(void);

/* Read the tick counter at a busy-poll site (a loop that spins on
 * timer_ticks without calling a wait helper).  Real-time mode: the plain
 * value.  Manual mode: also advances one tick so virtual time flows. */
dos_ulong timer_poll(void);

/* src/TIMER.C helpers, semantics unchanged (n is a signed 16-bit int that is
 * sign-extended before the unsigned 32-bit add, exactly like `cwd`). */
void    timer_wait_ticks(dos_int n);
void    timer_deadline_arm(dos_int n);
void    timer_deadline_wait(void);
dos_int timer_deadline_reached(void);

/* Historical install/restore entry points (boot_init_seed_rand /
 * game_shutdown).  In the port they start/stop the platform tick thread. */
void timer_irq_install(void);
void timer_irq_restore(void);

/* ---- platform hooks (implemented in portable/platform/sdl3/clock.c, or by
 * tests) ---- */
/* Start/stop the real-time tick source that calls timer_service_tick(). */
void timer_platform_start(void);
void timer_platform_stop(void);
/* Block the calling (game) thread until timer_ticks has changed, or return
 * immediately in manual mode.  Used by the wait helpers instead of a hot spin. */
void timer_platform_wait_tick(void);
/* Optional observer invoked after every tick (on the ticking thread):
 * deterministic replays hook their scripted input / frame dumps here. */
typedef void (*timer_tick_observer_fn)(void);
void timer_set_tick_observer(timer_tick_observer_fn fn);
/* Optional observer invoked (on the game thread) when timer_deadline_wait()
 * is entered, i.e. at the end of a game frame, with the current tick and
 * the armed deadline: the frame-interpolation presenter publishes the
 * finished frame here. */
typedef void (*timer_frame_observer_fn)(dos_ulong now_ticks, dos_ulong deadline_ticks);
void timer_set_frame_observer(timer_frame_observer_fn fn);

/* Manual mode: no thread; tests advance time with timer_service_tick(). */
void timer_service_set_manual(bool manual);
bool timer_service_is_manual(void);

#endif
