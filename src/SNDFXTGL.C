#include "TBL.H"
extern int n_sel, f1, f2;
extern void sound_stop_reset(void), sound_voices_reset(void), music_resume_if_valid(void);
void sound_effects_toggle(void)
{
    if (f2) {
        sound_stop_reset(); sound_voices_reset();
        f2 = f1 = 0;
        if (n_sel >= 0) tbl[n_sel].f15 = tbl[n_sel].d13 = 0;
    } else {
        f2 = f1 = 1;
        if (n_sel >= 0) tbl[n_sel].f15 = tbl[n_sel].d13 = 1;
        music_resume_if_valid();
    }
}
