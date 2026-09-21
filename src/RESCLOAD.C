/* F_D5BA -- load one of two adjacent records into the shared staging buffer,
   but only when the cached index at DS:237E says it is not already there. */
extern void resource_load_record_into();
extern char far *gc5da;                 /* DS:C5DA offset, DS:C5DC segment */
/* The compiler emits this two-byte initializer in this module's _DATA. */
int music_track_handle = -1;                          /* DS:237E, the cached index */
extern int g1778;                       /* DS:1778, the mode */

void resource_record_cache_load(v)
register int v;
{
    if (music_track_handle != v) {
        if (g1778 == 0) resource_load_record_into(v, gc5da);
        else resource_load_record_into(v + 1, gc5da);
        music_track_handle = v;
    }
}
