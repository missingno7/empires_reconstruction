/* sound_stub.c -- placeholder sound-engine hooks for timer_service_tick()
 * and for every function portable/include/sound.h declares.
 *
 * portable/game/timer.c's INT 8 body (src/TIMER.C F_6BCF) reads
 * sound_request_count/sound_enabled/music_enabled and calls
 * sound_tick_entry() every tick; timer.c declares those three objects
 * itself (a local `extern`, not through a header) because
 * portable/generated/game_data.h does not yet expose them as individually
 * addressable symbols -- see sound.h's header comment for the full gap
 * writeup.  This file defines them here, plus no-op bodies for the rest of
 * sound.h's C-facing entry points and the opl_write/opl_detect backend
 * API, purely so empires_core (and anything that later calls into
 * sound.h) links before the real SOUND.ASM port
 * (portable/audio/sound_driver.c, Wave 4) exists.
 *
 * Remove this file (and the matching target_sources() line in
 * portable/audio/CMakeLists.txt) once portable/audio provides the real
 * sound_driver.c with these definitions -- sound.h's prototypes do not
 * need to change.
 */
#include "sound.h"

/* sound_request_count (DS:237C), sound_enabled (DS:176E) and music_enabled
 * (DS:1772) are initialized DATA objects defined by the generated
 * game_data.c; this stub only supplies the entry points. */

void sound_tick_entry(void)
{
}

void sound_backend_select_init(void)
{
}

void sound_voice_table_reload(dos_int v)
{
    (void)v;
}

void sound_voices_reset(void)
{
}

void sound_voices_disable_all(void)
{
}

void opl_register_write(dos_int reg, dos_int val)
{
    (void)reg;
    (void)val;
}

void stream_control_block_arm(dos_int n)
{
    (void)n;
}

void sound_stop_reset(void)
{
}

void opl_write(dos_uint reg, dos_uint val)
{
    (void)reg;
    (void)val;
}

dos_int opl_detect(void)
{
    return 0;
}
