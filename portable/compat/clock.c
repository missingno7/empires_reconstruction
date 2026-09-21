/* clock.c -- real-time tick source for timer.h (portable/compat: host clock, no SDL; see
 * docs/portable/architecture.md "Timing model").
 *
 * Spawns a dedicated thread that calls timer_service_tick() at exactly
 * TIMER_TICK_HZ = 1193182 / 0x13B1 (~236.6975 Hz), the historical PIT rate.
 * This file lives under platform/sdl3 because the real-time tick policy is
 * host-specific, not because it needs SDL: it includes only timer.h and
 * sync.h (no <SDL3/SDL.h>), so portable/tests/test_clock.c can link it
 * directly without pulling in SDL or a window.
 */
#include "timer.h"
#include "sync.h"

#include <stdbool.h>
#include <stdint.h>

/* Exact tick period in nanoseconds = 1e9 * TIMER_PIT_DIVISOR / 1193182,
 * kept as an exact fraction (whole ns per tick + a numerator/PERIOD_DEN
 * remainder) instead of one multiply-by-tick-index, so the running deadline
 * is advanced by repeated integer addition with no overflow no matter how
 * long the thread runs. */
#define PERIOD_NUM ((uint64_t)1000000000ull * (uint64_t)TIMER_PIT_DIVISOR)
#define PERIOD_DEN ((uint64_t)1193182ull)

#define CLOCK_SPIN_NS         1000000ull /* spin-wait inside the last ~1ms of a deadline */
#define CLOCK_SLEEP_CHUNK_NS  2000000ull /* sleep in <=2ms chunks so a stop request is noticed promptly */
#define CLOCK_MAX_CATCHUP     8          /* at most this many back-to-back catch-up ticks before re-basing */
#define CLOCK_WAIT_TIMEOUT_MS 2u         /* timer_platform_wait_tick() cond-wait timeout, to be robust */

static sync_thread s_thread;
static sync_mutex  s_mutex;
static sync_cond   s_cond;
static bool        s_sync_inited     = false;
static volatile bool s_running       = false; /* thread believed alive (touched by the game/main thread only) */
static bool         s_stop_requested = false; /* guarded by s_mutex */

static void ensure_sync_objects(void)
{
    if (!s_sync_inited) {
        sync_mutex_init(&s_mutex);
        sync_cond_init(&s_cond);
        s_sync_inited = true;
    }
}

static bool stop_requested(void)
{
    bool stop;
    sync_mutex_lock(&s_mutex);
    stop = s_stop_requested;
    sync_mutex_unlock(&s_mutex);
    return stop;
}

/* Advance a tick deadline by exactly one period, carrying the exact
 * fractional remainder in *accum (see PERIOD_NUM/PERIOD_DEN comment). */
static void advance_deadline(uint64_t *deadline_ns, uint64_t *accum)
{
    *deadline_ns += PERIOD_NUM / PERIOD_DEN;
    *accum += PERIOD_NUM % PERIOD_DEN;
    if (*accum >= PERIOD_DEN) {
        *deadline_ns += 1;
        *accum -= PERIOD_DEN;
    }
}

static void do_tick(void)
{
    timer_service_tick();
    sync_mutex_lock(&s_mutex);
    sync_cond_broadcast(&s_cond);
    sync_mutex_unlock(&s_mutex);
}

static void tick_thread_fn(void *arg)
{
    (void)arg;
    uint64_t deadline_ns = sync_now_ns();
    uint64_t accum = 0;
    advance_deadline(&deadline_ns, &accum); /* first tick fires one period after start */

    while (!stop_requested()) {
        uint64_t now = sync_now_ns();

        if (now >= deadline_ns) {
            /* Behind schedule (e.g. the process was paused by a debugger).
             * Issue up to CLOCK_MAX_CATCHUP ticks back-to-back, with no
             * sleeping/spinning between them, so timer_ticks catches up
             * quickly without stalling the caller of timer_platform_start().
             * If still behind after that, re-base the deadline on the
             * current clock and drop the rest of the backlog instead of
             * free-running ticks forever -- documented behaviour, not a bug:
             * a long pause loses real elapsed ticks rather than firehosing
             * timer_ticks to "catch up" to wall-clock time. */
            int catchup = 0;
            while (now >= deadline_ns && catchup < CLOCK_MAX_CATCHUP && !stop_requested()) {
                do_tick();
                advance_deadline(&deadline_ns, &accum);
                catchup++;
                now = sync_now_ns();
            }
            if (now >= deadline_ns) {
                deadline_ns = now;
                accum = 0;
                advance_deadline(&deadline_ns, &accum);
            }
            continue;
        }

        /* Ahead of schedule: sleep in short chunks down to ~1ms of the
         * deadline (OS sleep granularity is coarse), checking the stop flag
         * between chunks, then spin-wait the last ~1ms for precision. */
        while (!stop_requested()) {
            now = sync_now_ns();
            if (now + CLOCK_SPIN_NS >= deadline_ns)
                break;
            uint64_t remaining = deadline_ns - now - CLOCK_SPIN_NS;
            sync_sleep_ns(remaining < CLOCK_SLEEP_CHUNK_NS ? remaining : CLOCK_SLEEP_CHUNK_NS);
        }
        if (stop_requested())
            break;
        while (sync_now_ns() < deadline_ns) {
            /* spin for the last fraction of a millisecond */
        }

        do_tick();
        advance_deadline(&deadline_ns, &accum);
    }
}

void timer_platform_start(void)
{
    ensure_sync_objects();
    sync_mutex_lock(&s_mutex);
    s_stop_requested = false;
    sync_mutex_unlock(&s_mutex);
    if (sync_thread_start(&s_thread, tick_thread_fn, NULL))
        s_running = true;
}

void timer_platform_stop(void)
{
    if (!s_running)
        return;
    sync_mutex_lock(&s_mutex);
    s_stop_requested = true;
    sync_mutex_unlock(&s_mutex);
    sync_thread_join(&s_thread);
    s_running = false;
}

void timer_platform_wait_tick(void)
{
    ensure_sync_objects();
    sync_mutex_lock(&s_mutex);
    sync_cond_wait_ms(&s_cond, &s_mutex, CLOCK_WAIT_TIMEOUT_MS);
    sync_mutex_unlock(&s_mutex);
}
