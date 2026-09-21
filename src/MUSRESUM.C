extern int music_track_handle;
extern void f_c7cb();
void music_resume_if_valid(void)
{
    if (music_track_handle >= 0) f_c7cb(0);
}
