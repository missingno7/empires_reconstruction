/* src/INTRO.C: Intro chapter driver and animation steps.
   One translation unit; the sections below were the separate member
   sources of grouped module C_5321_56C6 and keep their original ids. */

#include "DIALOG.H"
#include "VIDEO.H"
#include "SOUND.H"
/*@SYM _dialog_run=0x86C9 kind=f key=functions/F_86C9.entry*/

struct E { int f0, f2, f4, f6, f8, fa, fc, fe; };
struct P { int a, b; };

extern void intro_animate_step(struct E far *ev, int step, struct P far *q, int far *idx,
                  unsigned long far *when, int dx0, int dy0,
                  int far *ph, int far *pi, int far *pj, int far *pk);
extern void gfx_wipe_rect(), gfx_box(), gfx_copy_rect();
extern void sound_stop_reset(void);
extern void stream_control_block_arm(int n);
extern unsigned long timer_ticks;              /* DS:0B76 */
extern int resource_load_record();
extern char far *ui_gfx_shadow_a;                 /* DS:C5C6 offset, DS:C5C8 segment */
extern int timer_deadline_reached();
extern int keyboard_poll_nonblocking();
extern int keyboard_read_blocking_hotkeys();
extern int g8fc;                        /* DS:08FC */
extern void resource_load_record_alloc(int n, char far * far *p);   /* 684A */
/*@SYM _resource_load_record_alloc=0x684A kind=f key=functions/F_684A.entry*/
extern unsigned far *gbfde;             /* DS:BFDE offset, DS:BFE0 segment */
extern char far *gbfee[];               /* DS:BFEE offset, DS:BFF0 segment */
extern char far *resource_stripe_table;                 /* DS:99D2 offset, DS:99D4 segment */
extern void hud_prompt_continue_clear();
extern void anim_step_loop(int a,int b,int c,int d,int e,int f); /* 9F40 */
/*@SYM _anim_step_loop=0x9F40 kind=f key=functions/F_9F40.entry*/
extern void bitmap_blit_topleft(void);                 /* 5673 */
extern void hud_prompt_continue_draw(void);                 /* 75F3 */
extern int  g857, g94, g96, g98, g9a, g1774, g1776;
extern unsigned char vmode;              /* DS:BFCD */
/*@SYM _vmode=0xBFCD kind=g key=storage_objects/M_2BAFD.phys*/
extern char far *t1, far *t2, far *t3, far *t4;   /* BFE2, BFE6, BFDE, BFF2 */
/*@SYM _t1=0xBFE2 kind=g key=storage_objects/M_2BB12.phys*/
/*@SYM _t2=0xBFE6 kind=g key=storage_objects/M_2BB16.phys*/
/*@SYM _t3=0xBFDE kind=g key=storage_objects/M_2BB0E.phys*/
/*@SYM _t4=0xBFF2 kind=g key=storage_objects/M_2BB22.phys*/
extern char buf[];                       /* DS:BFEE */
/*@SYM _buf=0xBFEE kind=g key=storage_objects/REGION_2BB1E.phys*/
extern void gfx_color_select(int n);                 /* 01CE */
/*@SYM _gfx_color_select=0x01CE kind=f key=functions/F_01CE.entry*/
extern void clear(int a,int b,int c,int d);   /* 03A8 */
/*@SYM _clear=0x03A8 kind=f key=functions/F_03A8.entry*/
extern void box(int a,int b,int c,int d);     /* 039F */
/*@SYM _box=0x039F kind=f key=functions/F_039F.entry*/
extern void resource_record_cache_load(int n);                /* D5BA */
extern void splash_draw_and_clear(void);                 /* 555B */
extern void resource_ptr_table_build(void);                 /* 55C7 */
extern void copy(int a,int b,char far *s,int n);   /* 03CC */
/*@SYM _copy=0x03CC kind=f key=functions/F_03CC.entry*/
extern void resource_record_cache_reset(int n);                /* D5F9 */
extern void intro_play_script(struct E far *ev, int count, int step, struct P far *q);  /* 5321 */
/*@SYM _intro_play_script=0x5321 kind=f key=functions/F_5321.entry*/
extern void timer_deadline_arm(int n);                /* 6C57 */
/*@SYM _timer_deadline_arm=0x6C57 kind=f key=functions/F_6C57.entry*/
extern int  intro_wait_key(void);                /* 5593 */
/*@SYM _intro_wait_key=0x5593 kind=f key=functions/F_5593.entry*/
extern void text_draw_wrapped(int a,int b,char far *s);   /* 6D3C */
extern void f568c(void);                 /* 568C */
extern void timer_wait_ticks(int n);                /* 6C26 */
extern void farfree(char far *p);            /* F6C3 */
/*@SYM _farfree=0xF6C3 kind=f key=functions/F_F6C3.entry*/

/*@SYM _g139d=0x139D kind=g key=storage_objects/G_P20ECD.phys*/
/* Storage belongs to DATA_010FA5_RECORDS; this module only references it. */
extern struct dialog near g139d;

/* ---- F_5321 (original code at 0x5321) ---- */
/* F_5321 -- intro animation driver: replay `count` script steps through intro_animate_step.
   Typed probe of the six-word interface. */
void intro_play_script(struct E far *ev, int count, int step, struct P far *q)
{
    int idx;
    unsigned long when;
    int x, y, w, h;                /* previous sprite box; w==0 means none yet */
    idx = 0;
    when = 0;
    w = 0;
    while (idx < count)
        intro_animate_step(ev, step, q, &idx, &when, 0, 0, &x, &y, &w, &h);
}


/* ---- F_5382 (original code at 0x5382) ---- */
/* F_5382 -- the intro's animation-step service.  Entry 15482, 473 bytes,
   executed 40,061,167 times on the corpus (the hottest function the intro
   chapter owns).  Eleven parameters, eleven int locals, two register
   variables (SI = the slot index, DI = x).

   Shape, read off assets/AEPROG.EXE (ndisasm -b16 -o 0x5382):

     bp+04  struct E far *ev   16-byte script slots (`mov cl,4; shl ax,cl`)
     bp+08  int  step
     bp+0A  struct P far *q    4-byte pairs (`shl ax,1; shl ax,1`)
     bp+0E  int far *idx
     bp+12  unsigned long far *when
     bp+16  int  dx0
     bp+18  int  dy0
     bp+1A  int far *ph
     bp+1E  int far *pi
     bp+22  int far *pj
     bp+26  int far *pk

   5382  55 8BEC 83EC16 5657   prologue, 0x16 = eleven int locals
   538A  C45E12 268B5702 268B07 3B16780B 720B 7706 3B06760B 7603 E9B001
                                *when > timer_ticks -- the 32-bit UNSIGNED compare
                                (jc / ja / jna), so both sides are unsigned
                                long; greater means jmp to the epilogue.
   53A5  C45E0E 268B37 26FF07  s = (*idx)++   (value first, then inc memory)
   53AE..543D                  eight separate statements, each recomputing
                                s*16 and reloading ev: TC 2.0 does no CSE.
                                [bp-0xE] is written and never read again.
   5440  8B46EA 3DFEFF 7432 3DFFFF 7402 EB36
                                a SWITCH, not an if-chain: the selector is
                                loaded ONCE into ax and each arm is a
                                `cmp ax,imm / jz`, and the chain is emitted
                                in ASCENDING case order (-2 before -1) while
                                the BODIES stay in source order (-1 at 544F,
                                -2 at 547A, default at 5485).  The last
                                arm's break is the give-away `EB00` at 5553,
                                a jump of displacement zero to the switch's
                                end, which here is also the epilogue.
   548B  8B46F6 F76608 99      n * step then CWD: the `mul` is a 16x16 int
                                multiply whose high word is thrown away by
                                the sign-extension that widens the int
                                result to the long that is added to timer_ticks.
   54EA  ...26FF7702 26FF37    q[cmd] is pushed as TWO words from ONE address
                                computation -- a 4-byte STRUCT passed BY
                                VALUE, not two separate int arguments, which
                                would each have recomputed les bx / add bx.
                                `add sp,0xA` = 5 words = 4 arguments. */

void intro_animate_step(ev, step, q, idx, when, dx0, dy0, ph, pi, pj, pk)
struct E far *ev;
int step;
struct P far *q;
int far *idx;
unsigned long far *when;
int dx0, dy0;
int far *ph, far *pi, far *pj, far *pk;
{
    int cmd;                            /* bp-16 */
    int y;                              /* bp-14 */
    int w;                              /* bp-12 */
    int h;                              /* bp-10 */
    int t;                              /* bp-0E, written, never read */
    int c;                              /* bp-0C */
    int n;                              /* bp-0A */
    int ox;                             /* bp-08 */
    int oy;                             /* bp-06 */
    int ow;                             /* bp-04 */
    int oh;                             /* bp-02 */
    register int s, x;                  /* si, di */

    if (*when > timer_ticks) return;
    s = (*idx)++;
    cmd = ev[s].f0;
    x = ev[s].f2 + dx0;
    y = ev[s].f4 + dy0;
    w = ev[s].f6;
    h = ev[s].f8;
    t = ev[s].fa;
    c = ev[s].fc;
    n = ev[s].fe;
    switch (cmd) {
    case -1:
        gfx_wipe_rect(x, y + 0xc8, w, h, x, y);
        gfx_box(x, y, w, h);
        break;
    case -2:
        sound_stop_reset();
        stream_control_block_arm(x);
        break;
    default:
        if (n != 0)
            *when = n * step + timer_ticks;
        ox = *ph;
        oy = *pi;
        ow = *pj;
        oh = *pk;
        if (ow != 0)
            gfx_wipe_rect(ox, oy + 0xc8, ow, oh, ox, oy);
        gfx_copy_rect(x, y, q[cmd], c);
        if (ow != 0)
            gfx_box(ox, oy, ow, oh);
        gfx_box(x, y, w, h);
        *ph = x;
        *pi = y;
        *pj = w;
        *pk = h;
        break;
    }
}


/* ---- F_555B (original code at 0x555B) ---- */
/* F_555B -- read record 0x33, draw it at the origin and then clear the
   320x200 frame. */

void splash_draw_and_clear()
{
    resource_load_record(0x33);
    gfx_blit_bitmap(0, 0, ui_gfx_shadow_a);
    gfx_wipe_rect(0, 0, 0x140, 0xc8, 0, 0xc8);
}


/* ---- F_5593 (original code at 0x5593) ---- */
/* F_5593 -- wait for a key while the deadline at F_6C87 has not passed.
   The first test is `cmp ax,0Dh`, three bytes against the CALL RESULT, not
   the five-byte `cmp [_g8fc],0Dh` two statements would give: the assignment
   is the tested expression (rule 4 in its value-of-assignment form), and the
   SECOND test reloads from memory.  The dead `jmp` after `return 0x0D` is
   the `if` statement's own end jump, which TC 2.0 emits only when the `if`
   HAS an `else`. */
int intro_wait_key()
{
    while (timer_deadline_reached() == 0) {
        if (keyboard_poll_nonblocking()) {
            if ((g8fc = keyboard_read_blocking_hotkeys()) == 0x0d)
                return (0x0d);
            else if (g8fc == 0x1b)
                return (g8fc);
        }
    }
    return (-1);
}


/* ---- F_55C7 (original code at 0x55C7) ---- */
/* F_55C7 -- build the 22-entry far pointer table at DS:BFEE.  The first nine
   entries index the record loaded at DS:BFDE through its own leading word
   table; the next nine and the last four are fixed 674-byte strides into the
   block at DS:99D2.  The `push cx / push bx ... pop ax / pop dx` around each
   index computation is the ORIGINAL far-pointer shape of rule 3.  The two
   later loops increment the TABLE index before the loop counter, so the
   counter is the `for` variable and the table index rides in the comma. */
void resource_ptr_table_build()
{
    register int i;
    register int j;

    resource_load_record_alloc(0x34, &gbfde);
    for (i = 0; i < 9; i++)
        gbfee[i] = (char far *) gbfde + gbfde[i] + 2;
    i = 9;
    for (j = 0; j < 9; i++, j++)
        gbfee[i] = resource_stripe_table + j * 674;
    i = 0x12;
    for (j = 0x0c; j < 0x10; i++, j++)
        gbfee[i] = resource_stripe_table + j * 674;
}


/* ---- F_5673 (original code at 0x5673) ---- */
/* F_5673 -- blit the far bitmap at DS:BFEE to (0,0x20) with mode 0.  The far
   pointer GLOBAL pushes its segment word then its offset word (rule 5). */
void bitmap_blit_topleft()
{
    gfx_copy_rect(0, 0x20, gbfee[0], 0);
}


/* ---- F_568C (original code at 0x568C) ---- */
void f568c(void) { hud_prompt_continue_clear(); anim_step_loop(0,232,200,152,0,32); bitmap_blit_topleft(); hud_prompt_continue_draw(); gfx_box(0,0,320,200); }


/* ---- F_56C6 (original code at 0x56C6) ---- */
/* F_56C6 -- intro chapter driver.
   Both confirmation prompts use the shared g139d dialog descriptor inside
   DATA_010FA5_RECORDS (kind 7). Declare the shared storage instead of
   manufacturing a module-local initializer. */

int intro_run_chapter(void)
{
    int key;
    register int again, k;

    again = 1;
    if (g857 == 0) {
        g96 = 0x18f; g98 = 0; g9a = 0xa0;
        gfx_color_select(0);
        clear(0, 0, 0x140, 0xc8);
        resource_record_cache_load(0x35);
        splash_draw_and_clear();
        resource_load_record_alloc(0x38, &t1);
        resource_load_record_alloc(0x37, &t2);
        resource_ptr_table_build();
        copy(0xec, 0x89, t4, 0);
        bitmap_blit_topleft();
        box(0, 0, 0x140, 0xc8);
        g1776 = 1;
        resource_record_cache_reset(0x35);
        intro_play_script((struct E far *)(t1 + 2), 0x34, 0x1e, (struct P far *)buf);
        hud_prompt_continue_draw();
top:
        while (again) {
                timer_deadline_arm(0x1bc6);
                key = intro_wait_key();
                switch (key) {
                case 0x1b: if (dialog_run(&g139d) == 1) return -1; break;
                case -1:
                case 0x0d: again = 0;
                default:   break;
                }
            }
            again = 1;
            for (k = 0; k < 2; k++) {
                anim_step_loop(0, 0xe8, 0xc8, 0x8a, 0, 0x20);
                if (vmode == 2) gfx_color_select(5); else gfx_color_select(0xf);
                text_draw_wrapped(8, 0x1e, t2 + ((int far *)t2)[k] + 2);
                hud_prompt_continue_draw();
                box(0, 0, 0x140, 0xc8);
                while (again) {
                    timer_deadline_arm(0x1bc6);
                    key = intro_wait_key();
                    switch (key) {
                    case 0x1b: if (dialog_run(&g139d) == 1) return -1; break;
                    case -1:
                    case 0x0d: again = 0;
                default:   break;
                    }
                }
                again = 1;
            }
        if (k == 2 && key == -1) { f568c(); goto top; }
        anim_step_loop(0, 0x17e, 0xbc, 0x10, 0, 0xb6);
        g94 = 0; g9a = 0x9f; g1774 = 1;
        stream_control_block_arm(0x18);
        intro_play_script((struct E far *)(t1 + 0x344), 0xa, 0x2d, (struct P far *)buf);
        snd_on = 1;
        sound_stop_reset();
        g1774 = 0;
        timer_wait_ticks(0xed);
        intro_play_script((struct E far *)(t1 + 0x498), 0x16, 0x1e, (struct P far *)buf);
        intro_play_script((struct E far *)(t1 + 0x3e6), 0xb, 0x28, (struct P far *)buf);
        timer_wait_ticks(0xed);
        g1774 = 1;
        stream_control_block_arm(0x18);
        intro_play_script((struct E far *)(t1 + 0x66c), 0xb, 0x28, (struct P far *)buf);
        g1774 = 0;
        snd_on = 1;
        sound_stop_reset();
        intro_play_script((struct E far *)(t1 + 0x5fa), 7, 0x1e, (struct P far *)buf);
        g94 = 0x10; g96 = 0x9f; g98 = 4; g9a = 0x9b;
        farfree(t1); farfree(t2); farfree(t3);
    } else {
        gfx_color_select(0);
        clear(0, 0, 0x140, 0xc8);
        box(0, 0, 0x140, 0xc8);
    }
    return 1;
}
