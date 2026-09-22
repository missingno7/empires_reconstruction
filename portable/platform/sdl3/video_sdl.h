/* video_sdl.h -- SDL3 window/renderer/texture wrapper.
 *
 * This is the presentation half of the platform layer.  Its public API is
 * deliberately expressed in plain uint8_t buffers, independent of gfx.h:
 * once portable/gfx exists, the caller passes gfx_vram/gfx_dac straight
 * through without this header (or main.c) ever including gfx.h.
 *
 * Only files in portable/platform/sdl3/ may include <SDL3/SDL.h>.
 */
#ifndef PORTABLE_PLATFORM_SDL3_VIDEO_SDL_H
#define PORTABLE_PLATFORM_SDL3_VIDEO_SDL_H

#include <stdbool.h>
#include <stdint.h>

/* Logical (pre-scale) framebuffer size: historical mode 13h, 320x200 8bpp. */
#define SDL_VIDEO_LOGICAL_W 320
#define SDL_VIDEO_LOGICAL_H 200

/* Create the window, renderer and streaming texture.  `title` is used for
 * the window title.  Returns false and leaves nothing allocated on failure
 * (call sdl_video_shutdown() unconditionally afterwards regardless). */
bool sdl_video_init(const char *title);

/* Destroy texture/renderer/window and call SDL_Quit().  Safe to call after
 * a failed or partial sdl_video_init(), and safe to call more than once. */
void sdl_video_shutdown(void);

/* Expand an 8bpp indexed 320x200 buffer through a 256-entry, 3-channel,
 * 6-bit-per-channel DAC palette (values 0..63) into the streaming texture,
 * then present it (integer-scaled, nearest-neighbour) to the window.
 *
 * vram8: SDL_VIDEO_LOGICAL_W * SDL_VIDEO_LOGICAL_H bytes, row-major, one
 *        palette index per pixel.
 * dac6:  256 * 3 bytes, RGB triples, each component in 0..63.
 */
void sdl_video_present(const uint8_t *vram8, const uint8_t *dac6);

/* Toggle borderless fullscreen (Alt+Enter); the integer-scaled logical
 * presentation keeps the original pixels square. */
void sdl_video_toggle_fullscreen(void);
void sdl_video_set_fullscreen(bool full);

/* Pump the SDL event queue.  Returns true once a quit has been requested
 * (SDL_EVENT_QUIT, window close, or the Escape key). */
bool sdl_video_poll_events(void);

#endif /* PORTABLE_PLATFORM_SDL3_VIDEO_SDL_H */
