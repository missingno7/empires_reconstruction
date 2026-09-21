/* F_9F40 -- 0x10-step animation loop; the blit call is scaled by the video
   mode in display_mode.  q is the register copy of the third parameter. */
extern void timer_deadline_wait(void);
extern void timer_deadline_arm();
extern void f9ec3(), f039f();
extern char display_mode;

void anim_step_loop(int a, int b, int c, int d, int e, int f)
{
    register int i, q;

    q = c;
    for (i = 0; i < 0x10; i++) {
        timer_deadline_arm(9);
        if (display_mode == 5)
            f9ec3(a, b, q, d, e, f, i, 0x140);
        else if (display_mode == 2)
            f9ec3(a / 4, b, q / 4, d, e / 4, f, i, 0x50);
        else
            f9ec3(a / 2, b, q / 2, d, e / 2, f, i, 0xa0);
        f039f(e, f, q, d);
        timer_deadline_wait();
    }
}
