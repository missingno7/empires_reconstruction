/* input_sdl.h -- SDL3 keyboard event -> input_key_event() translation.
 *
 * Only files in portable/platform/sdl3/ may include <SDL3/SDL.h>.
 */
#ifndef PORTABLE_PLATFORM_SDL3_INPUT_SDL_H
#define PORTABLE_PLATFORM_SDL3_INPUT_SDL_H

#include <SDL3/SDL.h>

/* Translate one SDL keyboard event into an IBM PC set-1 scancode plus a
 * BIOS INT 16h-style ASCII value and forward it to input_key_event()
 * (portable/include/input.h).  Events other than SDL_EVENT_KEY_DOWN/
 * SDL_EVENT_KEY_UP, and scancodes with no set-1 mapping, are ignored.
 * Auto-repeat (e->key.repeat) is not filtered: every KEY_DOWN becomes
 * another make event, matching input.h's "Repeats are delivered as
 * additional make events." */
void input_sdl_handle_event(const SDL_Event *e);

#endif /* PORTABLE_PLATFORM_SDL3_INPUT_SDL_H */
