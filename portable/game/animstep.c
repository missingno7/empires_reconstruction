/* src/ANIMSTEP.C: 0x10-step animation loop. */
#include "game.h"

/* ---- F_9F40 (original code at 0x9F40) ---- */
/* F_9F40 -- 0x10-step animation loop; the blit call is scaled by the video
   mode in display_mode.  q is the register copy of the third parameter. */
void anim_step_loop(dos_int a, dos_int b, dos_int c, dos_int d, dos_int e, dos_int f)
{
    dos_int i, q;

    q = c;
    for (i = 0; i < 0x10; i++) {
        if (intro_skip_poll())
            return;
        timer_deadline_arm(9);
        if (display_mode == 5)
            anim_step_row_copy(a, b, q, d, e, f, i, 0x140);
        else if (display_mode == 2)
            anim_step_row_copy(a / 4, b, q / 4, d, e / 4, f, i, 0x50);
        else
            anim_step_row_copy(a / 2, b, q / 2, d, e / 2, f, i, 0xa0);
        gfx_box(e, f, q, d);
        timer_deadline_wait();
    }
}
