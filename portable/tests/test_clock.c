/* test_clock.c -- measures the SDL3 platform tick thread's real rate.
 *
 * Links ../platform/sdl3/clock.c directly (see portable/tests/timer.cmake),
 * so this executable does not require EMPIRES_BUILD_APP or SDL3: clock.c
 * itself never includes <SDL3/SDL.h> (see its header comment).
 */
#include "timer.h"
#include "sync.h"

#include <stdio.h>

int main(void)
{
    timer_service_set_manual(false);
    timer_ticks = 0;

    timer_platform_start();
    sync_sleep_ns(500000000ull); /* ~500ms of real time */
    timer_platform_stop();

    dos_ulong ticks = timer_ticks;
    double expected = 0.5 * TIMER_TICK_HZ; /* TIMER_TICK_HZ = 1193182/0x13B1 ~= 236.6975 Hz */
    double measured_hz = (double)ticks / 0.5;

    printf("test_clock: %lu ticks in ~500ms -- measured %.4f Hz, expected %.4f Hz (%.4f ticks)\n",
           (unsigned long)ticks, measured_hz, TIMER_TICK_HZ, expected);

    double lower = expected * 0.97;
    double upper = expected * 1.03;
    if ((double)ticks < lower || (double)ticks > upper) {
        fprintf(stderr,
                "test_clock: tick count %lu outside +/-3%% of expected %.4f (range [%.4f, %.4f])\n",
                (unsigned long)ticks, expected, lower, upper);
        return 1;
    }
    return 0;
}
