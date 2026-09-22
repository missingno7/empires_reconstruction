/* sound_stub.c -- retired.
 *
 * Every function portable/include/sound.h declares (the 8 asm/SOUND.ASM
 * C-facing entry points, opl_write/opl_detect, and the 4 sound_backend_*
 * event hooks) is now implemented by portable/audio/sound_driver.c (Phase
 * 12 part 1 -- the real SOUND.ASM port). sound_request_count/
 * sound_enabled/music_enabled are likewise real generated DATA objects
 * (portable/generated/game_data.c) now, not ad hoc globals defined here.
 *
 * Kept (rather than deleted) per this phase's brief ("keep the file for
 * anything still unported") -- nothing in portable/include/sound.h remains
 * unported, so this only anchors the translation unit (same pattern as
 * portable/compat/dos_compat.c's own placeholder anchor).
 */
#include "dos_types.h"
int sound_stub_link_anchor(void) { return 0; }
