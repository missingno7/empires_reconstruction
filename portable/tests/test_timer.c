/* test_timer.c -- manual-mode unit tests for timer.h / portable/game/timer.c
 * (src/TIMER.C port).
 *
 * Links empires_core only, NOT clock.c (see portable/tests/timer.cmake).
 * timer.c calls timer_platform_start()/timer_platform_stop()/
 * timer_platform_wait_tick() as extern hooks -- timer.h documents them as
 * "implemented in portable/platform/sdl3/clock.c, or by tests" -- so this
 * file supplies trivial stand-ins purely to satisfy the linker.  Manual mode
 * (set below) never actually calls timer_platform_wait_tick() -- timer.c's
 * wait loops call timer_service_tick() directly instead when manual -- and
 * this test never calls timer_irq_install()/timer_irq_restore(), so none of
 * the three stand-in bodies are exercised at runtime.
 */
#include "timer.h"

#include <stdio.h>

/* The real hooks now live in portable/compat/clock.c (linked from
 * empires_core); manual mode never calls them. */

static int s_failures = 0;

#define CHECK(cond) \
    do { \
        if (!(cond)) { \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
            s_failures++; \
        } \
    } while (0)

static void test_tick_stepping(void)
{
    timer_ticks = 0;
    timer_service_tick();
    CHECK(timer_ticks == 1);
    timer_service_tick();
    timer_service_tick();
    CHECK(timer_ticks == 3);
}

static void test_wait_ticks_advances(void)
{
    timer_ticks = 100;
    timer_wait_ticks(5); /* manual mode: steps timer_service_tick() 5 times */
    CHECK(timer_ticks == 105);
}

static void test_wait_ticks_zero(void)
{
    timer_ticks = 50;
    timer_wait_ticks(0); /* deadline already met: must not tick at all */
    CHECK(timer_ticks == 50);
}

static void test_deadline_arm_wait_reached(void)
{
    timer_ticks = 10;
    timer_deadline_arm(4);
    CHECK(gc0d0 == 14);
    CHECK(timer_deadline_reached() == 0);
    timer_deadline_wait();
    CHECK(timer_ticks == 14);
    CHECK(timer_deadline_reached() == 1);
}

/* F_6C26/F_6C57: n is a signed 16-bit dos_int, sign-extended with `cwd`
 * before the unsigned 32-bit add (see timer.c's comment on the cast chain).
 * A zero-extension bug would turn n=-5 into 0xFFFB (65531), making the wait/
 * arm land tens of thousands of ticks in the future instead of 5 ticks in
 * the past. */
static void test_sign_extension_negative_n(void)
{
    timer_ticks = 1000;
    timer_wait_ticks(-5); /* deadline = 995, already passed: must return without ticking */
    CHECK(timer_ticks == 1000);

    timer_ticks = 2000;
    timer_deadline_arm(-3);
    CHECK(gc0d0 == 1997);
    CHECK(timer_deadline_reached() == 1);
}

/* F_6C87 compares timer_ticks < gc0d0 UNSIGNED, literally -- src/TIMER.C
 * uses ja/jb/jae on the raw 32-bit halves.  Arming a deadline that wraps
 * past 0xFFFFFFFF makes timer_deadline_reached() report "reached" the whole
 * time the counter is still above the wrap point (it has not genuinely
 * caught up), and only stops reporting "reached" once timer_ticks itself
 * wraps and falls back below the (now small) deadline value.  This is the
 * historical quirk, preserved literally rather than fixed. */
static void test_wraparound_near_0xffffffff(void)
{
    timer_ticks = 0xFFFFFFF0u;
    timer_deadline_arm(0x20);
    CHECK(gc0d0 == 0x10u); /* 0xFFFFFFF0 + 0x20 wraps past 0 */
    CHECK(timer_deadline_reached() == 1); /* "before" real elapse: still reports reached (quirk) */

    timer_ticks = 0xFFFFFFFFu;
    CHECK(timer_deadline_reached() == 1); /* "after" advancing further, still short of the real +0x20: still reached */

    timer_ticks = 0x00000005u; /* counter has now wrapped past 0 */
    CHECK(timer_deadline_reached() == 0); /* genuinely not caught up to gc0d0=0x10 yet */
    timer_ticks = 0x00000010u;
    CHECK(timer_deadline_reached() == 1); /* genuinely caught up */
}

int main(void)
{
    timer_service_set_manual(true);
    CHECK(timer_service_is_manual());

    test_tick_stepping();
    test_wait_ticks_advances();
    test_wait_ticks_zero();
    test_deadline_arm_wait_reached();
    test_sign_extension_negative_n();
    test_wraparound_near_0xffffffff();

    if (s_failures) {
        fprintf(stderr, "test_timer: %d check(s) failed\n", s_failures);
        return 1;
    }
    printf("test_timer: all checks passed\n");
    return 0;
}
