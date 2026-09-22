/* src/INTRO.C: Intro chapter driver and animation steps. */
#include "game.h"
#include "gfx_tween.h"
#include "trace.h"

/* The original intro only looked at the BIOS key buffer at its prompt
 * boundaries.  Keep that behavior for every other part of the game, but let
 * the portable Space shortcut be observed by the animation loops too. */
static bool s_intro_active;
static bool s_intro_skip_requested;

dos_int intro_skip_poll(void)
{
    if (!s_intro_active)
        return 0;
    if (s_intro_skip_requested)
        return 1;
    if (keyboard_poll_nonblocking() == 0x20) {
        (void)keyboard_read_blocking_hotkeys();
        s_intro_skip_requested = true;
        return 1;
    }
    return 0;
}

dos_int intro_skip_requested(void)
{
    return s_intro_skip_requested ? 1 : 0;
}


/* PORT: `vmode` (src/INTRO.C local extern, "unsigned char vmode;" at DS:BFCD)
   is the historical local name for `display_mode` (portable/include/
   resource.h: "extern dos_char display_mode;" at the same DS:BFCD, "the
   port runs 4") -- same object, used directly instead of a local
   redeclaration. */

/* PORT (supervisor decision): `struct P {dos_int a,b;}` (game_funcs.h) is
   nothing but the two words of a far pointer; `q` is always `buf`/`gbfee`,
   the 22-entry far-pointer table, and `q[cmd]` passed by value IS the
   pointer `gbfee[cmd]`.  Ported as `dos_char **q` throughout instead --
   struct P is not referenced anywhere in this file any more.  game_funcs.h
   still declares `intro_play_script`/`intro_animate_step` with the old
   `struct P *q` parameter (not yet regenerated); the local prototypes
   below match the NEW signature and will conflict with game_funcs.h's
   declaration (a real, expected compile error) until it is regenerated to
   `dos_char **q` -- this is intentional per the supervisor's message, not
   an oversight. */
void intro_play_script(struct E *ev, dos_int count, dos_int step, dos_char **q);
void intro_animate_step(struct E *ev, dos_int step, dos_char **q, dos_int *idx, dos_ulong *when, dos_int dx0, dos_int dy0, dos_int *ph, dos_int *pi, dos_int *pj, dos_int *pk);

/* ---- F_5321 (original code at 0x5321) ---- */
/* F_5321 -- intro animation driver: replay `count` script steps through intro_animate_step.
   Typed probe of the six-word interface. */
void intro_play_script(struct E *ev, dos_int count, dos_int step, dos_char **q)
{
    dos_int idx;
    dos_ulong when;
    dos_int x, y, w, h;                /* previous sprite box; w==0 means none yet */
    idx = 0;
    when = 0;
    w = 0;
    while (idx < count) {
        if (intro_skip_poll())
            break;
        intro_animate_step(ev, step, q, &idx, &when, 0, 0, &x, &y, &w, &h);
    }
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
                                PORT: int-semantics-inventory.md fact 8:
                                `dos_ulong *when` / `dos_ulong timer_ticks`,
                                native unsigned compare, no helper needed.
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
                                PORT: int-semantics-inventory.md fact 9:
                                dos_mul16(n, step) truncated to 16 bits
                                first, THEN sign-extended (cwd) to dos_long
                                before adding the unsigned timer_ticks.
   54EA  ...26FF7702 26FF37    q[cmd] is pushed as TWO words from ONE address
                                computation -- a 4-byte STRUCT passed BY
                                VALUE, not two separate int arguments, which
                                would each have recomputed les bx / add bx.
                                `add sp,0xA` = 5 words = 4 arguments. */

void intro_animate_step(struct E *ev, dos_int step, dos_char **q, dos_int *idx,
                  dos_ulong *when, dos_int dx0, dos_int dy0,
                  dos_int *ph, dos_int *pi, dos_int *pj, dos_int *pk)
{
    dos_int cmd;                            /* bp-16 */
    dos_int y;                              /* bp-14 */
    dos_int w;                              /* bp-12 */
    dos_int h;                              /* bp-10 */
    dos_int t;                              /* bp-0E, written, never read */
    dos_int c;                              /* bp-0C */
    dos_int n;                              /* bp-0A */
    dos_int ox;                             /* bp-08 */
    dos_int oy;                             /* bp-06 */
    dos_int ow;                             /* bp-04 */
    dos_int oh;                             /* bp-02 */
    dos_int s, x;                           /* si, di */

    if (*when > timer_poll()) return;   /* busy-poll site: see timer_poll() */
    s = (*idx)++;
    cmd = ev[s].f0;
    x = ev[s].f2 + dx0;
    y = ev[s].f4 + dy0;
    w = ev[s].f6;
    h = ev[s].f8;
    t = ev[s].fa;
    c = ev[s].fc;
    n = ev[s].fe;
    (void)t;
    switch (cmd) {
    case -1:
        gfx_wipe_rect(x, y + 0xc8, w, h, x, y);
        gfx_box(x, y, w, h);
        timer_frame_boundary(*when);    /* frame interpolation: event done, next at *when */
        break;
    case -2:
        sound_stop_reset();
        stream_control_block_arm(x);
        break;
    default:
        if (n != 0)
            /* PORT: int-semantics-inventory.md fact 9 (n*step truncated to
               16 bits first, then sign-extended before the unsigned add). */
            *when = (dos_ulong)(dos_long)dos_mul16(n, step) + timer_ticks;
        ox = *ph;
        oy = *pi;
        ow = *pj;
        oh = *pk;
        if (ow != 0)
            gfx_wipe_rect(ox, oy + 0xc8, ow, oh, ox, oy);
        /* PORT (supervisor decision): q is dos_char **q (the flattened
           gbfee far-pointer table); q[cmd] is directly the bitmap pointer
           gbfee[cmd], not a byte-reinterpreted struct any more. */
        gfx_tween_tag = GFX_TWEEN_TAG_INTRO;     /* frame interpolation observer tag, see gfx_tween.h */
        gfx_copy_rect(x, y, (const uint8_t *)q[cmd], c);
        gfx_tween_tag = 0;
        if (ow != 0)
            gfx_box(ox, oy, ow, oh);
        gfx_box(x, y, w, h);
        timer_frame_boundary(*when);    /* frame interpolation: event done, next at *when */
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
dos_int intro_wait_key()
{
    while (timer_deadline_reached() == 0) {
        if (intro_skip_poll())
            return 0x20;
        if (keyboard_poll_nonblocking()) {
            if ((g8fc = keyboard_read_blocking_hotkeys()) == 0x0d)
                return (0x0d);
            else if (g8fc == 0x1b)
                return (g8fc);
            else if (g8fc == 0x20) {
                s_intro_skip_requested = true;
                return (g8fc); /* portable shortcut: skip to player sign-in */
            }
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
    dos_int i;
    dos_int j;

    /* PORT: `buf`/gbfee (src/INTRO.C:29, historical `char far *gbfee[]`) is
       now generated as `dos_char *buf[22]` with `#define gbfee buf` and
       `#define t4 (buf[1])` (generator fix landed, supervisor decision) --
       all 22 slots have real storage, plain indexing below. */
    resource_load_record_alloc(0x34, (uint8_t **)&gbfde);
    for (i = 0; i < 9; i++)
        gbfee[i] = (dos_char *) gbfde + gbfde[i] + 2;
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
    gfx_copy_rect(0, 0x20, (const uint8_t *)gbfee[0], 0);
}


/* ---- F_568C (original code at 0x568C) ---- */
void intro_title_picture_redisplay(void) { hud_prompt_continue_clear(); anim_step_loop(0,232,200,152,0,32); bitmap_blit_topleft(); hud_prompt_continue_draw(); gfx_box(0,0,320,200); }


/* ---- F_56C6 (original code at 0x56C6) ---- */
/* F_56C6 -- intro chapter driver.
   Both confirmation prompts use the shared g139d dialog descriptor inside
   DATA_010FA5_RECORDS (kind 7). Declare the shared storage instead of
   manufacturing a module-local initializer. */

dos_int intro_run_chapter(void)
{
    EMPIRES_TRACE("intro_run_chapter");
    dos_int key = 0;  /* PORT: MSVC /W4 cannot prove the `again=1;`-guarded
                          while loop below always runs at least once (it
                          does; every path sets `key` before it is read);
                          initialized only to silence C4701, no semantic
                          change. */
    dos_int again, k;

    again = 1;
    s_intro_active = (g857 == 0);
    s_intro_skip_requested = false;
    if (g857 == 0) {
        g96 = 0x18f; g98 = 0; g9a = 0xa0;
        gfx_color_select(0);
        gfx_clear_rect(0, 0, 0x140, 0xc8);
        resource_record_cache_load(0x35);
        splash_draw_and_clear();
        resource_load_record_alloc(0x38, (uint8_t **)&t1);
        resource_load_record_alloc(0x37, (uint8_t **)&t2);
        resource_ptr_table_build();
        gfx_copy_rect(0xec, 0x89, (const uint8_t *)buf[1], 0);   /* t4 == buf[1] */
        bitmap_blit_topleft();
        gfx_box(0, 0, 0x140, 0xc8);
        snd_flag2 = 1;
        resource_record_cache_reset(0x35);
        intro_play_script((struct E *)(t1 + 2), 0x34, 0x1e, gbfee);
        if (intro_skip_requested())
            goto skip_intro;
        hud_prompt_continue_draw();
top:
        while (again) {
                timer_deadline_arm(0x1bc6);
                key = intro_wait_key();
                switch (key) {
                case 0x1b: if (dialog_run(&g139d) == 1) { s_intro_active = false; return -1; } break;
                case 0x20: goto skip_intro;
                case -1:
                case 0x0d: again = 0; break;
                default:   break;
                }
            }
            again = 1;
            for (k = 0; k < 2; k++) {
                anim_step_loop(0, 0xe8, 0xc8, 0x8a, 0, 0x20);
                if (intro_skip_requested())
                    goto skip_intro;
                if (display_mode == 2) gfx_color_select(5); else gfx_color_select(0xf);
                text_draw_wrapped(8, 0x1e, t2 + ((dos_int *)t2)[k] + 2);
                hud_prompt_continue_draw();
                gfx_box(0, 0, 0x140, 0xc8);
                while (again) {
                    timer_deadline_arm(0x1bc6);
                    key = intro_wait_key();
                    switch (key) {
                    case 0x1b: if (dialog_run(&g139d) == 1) { s_intro_active = false; return -1; } break;
                    case 0x20: goto skip_intro;
                    case -1:
                    case 0x0d: again = 0; break;
                default:   break;
                    }
                }
                again = 1;
            }
        if (k == 2 && key == -1) {
            intro_title_picture_redisplay();
            if (intro_skip_requested())
                goto skip_intro;
            goto top;
        }
        anim_step_loop(0, 0x17e, 0xbc, 0x10, 0, 0xb6);
        if (intro_skip_requested())
            goto skip_intro;
        g94 = 0; g9a = 0x9f; mus_flag = 1;
        stream_control_block_arm(0x18);
        intro_play_script((struct E *)(t1 + 0x344), 0xa, 0x2d, gbfee);
        if (intro_skip_requested())
            goto skip_intro;
        snd_on = 1;
        sound_stop_reset();
        mus_flag = 0;
        timer_wait_ticks(0xed);
        intro_play_script((struct E *)(t1 + 0x498), 0x16, 0x1e, gbfee);
        if (intro_skip_requested())
            goto skip_intro;
        intro_play_script((struct E *)(t1 + 0x3e6), 0xb, 0x28, gbfee);
        if (intro_skip_requested())
            goto skip_intro;
        timer_wait_ticks(0xed);
        mus_flag = 1;
        stream_control_block_arm(0x18);
        intro_play_script((struct E *)(t1 + 0x66c), 0xb, 0x28, gbfee);
        if (intro_skip_requested())
            goto skip_intro;
        mus_flag = 0;
        snd_on = 1;
        sound_stop_reset();
        intro_play_script((struct E *)(t1 + 0x5fa), 7, 0x1e, gbfee);
        if (intro_skip_requested())
            goto skip_intro;
        g94 = 0x10; g96 = 0x9f; g98 = 4; g9a = 0x9b;
        free(t1); free(t2); free(gbfde);   /* t3 == gbfde */
        s_intro_active = false;
    } else {
        gfx_color_select(0);
        gfx_clear_rect(0, 0, 0x140, 0xc8);
        gfx_box(0, 0, 0x140, 0xc8);
    }
    return 1;

skip_intro:
    /* Space is a host convenience shortcut.  Stop intro audio and free the
     * chapter-owned allocations before continuing to slot_menu_run(). */
    EMPIRES_TRACE("intro_skip_to_player_sign_in");
    s_intro_active = false;
    sound_stop_reset();
    mus_flag = 0;
    snd_on = 1;
    g94 = 0x10; g96 = 0x9f; g98 = 4; g9a = 0x9b;
    free(t1); free(t2); free(gbfde);
    t1 = NULL;
    t2 = NULL;
    gbfde = NULL;
    return 1;
}
