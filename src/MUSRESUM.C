extern int music_track_handle;
extern void sound_voice_table_reload();
void music_resume_if_valid(void)
{
    if (music_track_handle >= 0) sound_voice_table_reload(0);
}
