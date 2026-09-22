/* audio_sdl.h -- SDL3 audio device glue (see audio_sdl.c).
 *
 * Only files in portable/platform/sdl3/ may include <SDL3/SDL.h>; this is
 * the ONLY file in that directory that opens an audio device (video_sdl.c
 * stays video-only).
 */
#ifndef PORTABLE_PLATFORM_SDL3_AUDIO_SDL_H
#define PORTABLE_PLATFORM_SDL3_AUDIO_SDL_H

#include <stdbool.h>

/* Initialise the PCM mixer (audio.h) and open an SDL3 audio device/stream
 * that pulls rendered PCM from it.  If the device fails to open, logs the
 * SDL error to stderr and returns false; the caller is expected to
 * continue running without audio (task requirement: "if the device fails
 * to open, log and continue silently" -- silently as in "the game keeps
 * running", not "without a diagnostic").  Call once, after
 * sdl_video_init(). */
bool audio_sdl_init(void);

/* Stop and close the audio device/stream (safe to call even if
 * audio_sdl_init() returned false or was never called) and shut down the
 * mixer.  Call once, at program exit. */
void audio_sdl_shutdown(void);

#endif /* PORTABLE_PLATFORM_SDL3_AUDIO_SDL_H */
