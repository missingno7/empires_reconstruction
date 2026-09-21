/* src/RESCACHE.C: Record cache load/reset and music resume.
   One translation unit; the sections below were the separate member
   sources of grouped module C_D5BA_D60C and keep their original ids. */

/* ---- F_D5BA (original code at 0xD5BA) ---- */
/* F_D5BA -- load one of two adjacent records into the shared staging buffer,
   but only when the cached index at DS:237E says it is not already there. */
extern void resource_load_record_into();
extern char far *gc5da;                 /* DS:C5DA offset, DS:C5DC segment */
/* The compiler emits this two-byte initializer in this module's _DATA. */
int music_track_handle = -1;                          /* DS:237E, the cached index */
#include "SOUND.H"

void resource_record_cache_load(v)
register int v;
{
    if (music_track_handle != v) {
        if (snd_backend_mode == 0) resource_load_record_into(v, gc5da);
        else resource_load_record_into(v + 1, gc5da);
        music_track_handle = v;
    }
}


/* ---- F_D5F9 (original code at 0xD5F9) ---- */
/* F_D5F9 -- forward and then clear. */
extern void resource_record_cache_load(), sound_voice_table_reload();

void resource_record_cache_reset(int n)
{
    resource_record_cache_load(n);
    sound_voice_table_reload(0);
}


/* ---- F_D60C (original code at 0xD60C) ---- */
extern int music_track_handle;
extern void sound_voice_table_reload();
void music_resume_if_valid(void)
{
    if (music_track_handle >= 0) sound_voice_table_reload(0);
}
