/* rescache.c -- portable port of src/RESCACHE.C: record cache load/reset
 * and music resume.
 * One translation unit; the sections below were the separate member
 * sources of grouped module C_D5BA_D60C and keep their original ids.
 */
#include "game.h"

/* ---- F_D5BA (original code at 0xD5BA) ---- */
/* Ported-C-owned DATA (DATA_011FAE_CACHED_INDEX, DS:237E, code_owner
 * F_D5BA per docs/portable/state-map.md): the cached record index.
 * Referenced (extern) by src/PUZZLE.C and src/SLOTMENU.C; no generated
 * header declares it, so those files' local `extern dos_int
 * music_track_handle;` stays necessary until this object gets a home in
 * game_state.h/game_data.h (flagged in the port report). */
dos_int music_track_handle = -1;

void resource_record_cache_load(dos_int v)
{
    if (music_track_handle != v) {
        if (snd_backend_mode == 0)
            resource_load_record_into((dos_uint)v, (uint8_t *)gc5da);
        else
            resource_load_record_into((dos_uint)(v + 1), (uint8_t *)gc5da);
        music_track_handle = v;
    }
}

/* ---- F_D5F9 (original code at 0xD5F9) ---- */
void resource_record_cache_reset(dos_int n)
{
    resource_record_cache_load(n);
    sound_voice_table_reload(0);
}

/* ---- F_D60C (original code at 0xD60C) ---- */
void music_resume_if_valid(void)
{
    if (music_track_handle >= 0)
        sound_voice_table_reload(0);
}
