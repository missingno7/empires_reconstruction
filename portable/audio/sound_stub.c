/* sound_stub.c -- placeholder sound-engine hooks for timer_service_tick().
 *
 * portable/game/timer.c's INT 8 body (src/TIMER.C F_6BCF) reads
 * sound_request_count/sound_enabled/music_enabled and calls
 * sound_tick_entry() every tick.  The real SOUND.ASM port
 * (portable/audio/sound_driver.c) is not written yet, so this file defines
 * the four symbols with historically-correct zero-initialized state and a
 * no-op tick entry, purely so empires_core links.
 *
 * Remove this file (and the matching target_sources() line in
 * portable/audio/CMakeLists.txt) once portable/audio provides the real
 * sound_driver.c with these definitions -- timer.c's `extern` declarations
 * do not need to change.
 */
#include "dos_types.h"

dos_int sound_request_count;
dos_int sound_enabled;
dos_int music_enabled;

void sound_tick_entry(void)
{
}
