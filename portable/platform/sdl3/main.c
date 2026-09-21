/* main.c -- SDL3 executable entry point (Milestone A skeleton).
 *
 * Creates a resizable, integer-scaled window and presents a test pattern
 * through video_sdl.h.  Once portable/gfx exists, gfx_vram/gfx_dac replace
 * the local test buffers below and get passed to sdl_video_present() as-is.
 *
 * Only files in portable/platform/sdl3/ may include <SDL3/SDL.h>.
 */
#include <SDL3/SDL.h>

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "video_sdl.h"

/* Selftest runs the loop for roughly this many milliseconds, then exits
 * cleanly with status 0 (used by CI / tests, no human required). */
#define SELFTEST_DURATION_MS 300

static uint8_t s_vram8[SDL_VIDEO_LOGICAL_W * SDL_VIDEO_LOGICAL_H];
static uint8_t s_dac6[256 * 3];

/* Fill the test buffers with a visible 16-colour gradient: the DAC's first
 * 16 entries ramp red/green up and blue down, and the framebuffer is split
 * into 16 vertical bands indexing those entries. */
static void build_test_pattern(void)
{
    memset(s_dac6, 0, sizeof(s_dac6));
    for (int i = 0; i < 16; ++i) {
        uint8_t level = (uint8_t)(i * 4); /* 0..60, within the 6-bit 0..63 DAC range */
        s_dac6[i * 3 + 0] = level;
        s_dac6[i * 3 + 1] = level;
        s_dac6[i * 3 + 2] = (uint8_t)(60 - level);
    }

    for (int y = 0; y < SDL_VIDEO_LOGICAL_H; ++y) {
        for (int x = 0; x < SDL_VIDEO_LOGICAL_W; ++x) {
            uint8_t band = (uint8_t)((x * 16) / SDL_VIDEO_LOGICAL_W);
            s_vram8[(size_t)y * SDL_VIDEO_LOGICAL_W + x] = band;
        }
    }
}

int main(int argc, char **argv)
{
    bool selftest = false;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--selftest") == 0) {
            selftest = true;
        }
    }

    bool ok = sdl_video_init("Empires (portable)");
    if (!ok) {
        sdl_video_shutdown();
        return 1;
    }

    build_test_pattern();

    Uint64 start_ticks = SDL_GetTicks();
    bool quit = false;
    while (!quit) {
        quit = sdl_video_poll_events();
        sdl_video_present(s_vram8, s_dac6);

        if (selftest && (SDL_GetTicks() - start_ticks) >= SELFTEST_DURATION_MS) {
            quit = true;
        }
    }

    sdl_video_shutdown();
    return 0;
}
