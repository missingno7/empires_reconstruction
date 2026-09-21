/* src/HUD.C: HUD panel, tabs, scrolling and the energy meter.
   One translation unit; the sections below were the separate member
   sources of grouped module C_6FC3_747B and keep their original ids. */

extern int gb83, gb85, gb80, gb7e, g0b7e, gc0fa, gc0fc, gc0ec;
extern char energy_meter, display_mode;
extern void f03c9();
extern void f039f();
extern void f03a8(int a, int b, int c, int d);
extern void f03b4();
extern void rect_border_draw();
extern void gfx_color_select(int n);
extern int f020f(void);
extern void hud_draw_meter(void);
extern void hud_tab_draw(void), energy_draw(void), f7417(void), f7443(void), hud_frame_draw(void);
extern int hud_prompt_select_draw();
extern void hud_prompt_continue_draw(), hud_prompt_message_draw();
extern int puzzle_piece_count();
extern int tick_div8();
extern int campaign_node_index();
extern void resource_load_record_alloc();
extern void tutorial_hint_dialog_show();
extern void f03a2(int a, int b, int c);
extern void f03a5(int a, int b, int c);

/* ---- F_6FC3 (original code at 0x6FC3) ---- */
hud_panel_clear() { gb83 = 0; }


/* ---- F_6FCA (original code at 0x6FCA) ---- */
/* F_6FCA -- load record 0x3F into the far block whose pointer lives at
   DS:C0EE.  The `push ds / mov ax,0C0EEh / push ax` pair is the ADDRESS of
   that pointer, F_684A's `char far * far *` OUT parameter. */
extern char far *gc0ee;                 /* DS:C0EE offset, DS:C0F0 segment */

void hud_icons_load()
{
    resource_load_record_alloc(0x3f, &gc0ee);
}


/* ---- F_6FDA (original code at 0x6FDA) ---- */
/* F_6FDA -- open the panel: blit its backdrop, draw the five widgets, frame
   it.  gc0ee is a far pointer whose first word is its own length. */
extern char far *gc0ee;

void hud_panel_open(void)
{
    gc0fa = 0;
    gb83 = 1;
    gc0fc = 1;
    f03c9(6, 0xa2, gc0ee + *(int far *) gc0ee + 2);
    hud_draw_meter();
    hud_tab_draw();
    energy_draw();
    f7417();
    f7443();
    f039f(6, 0xa2, 0x134, 0x24);
    hud_frame_draw();
    gc0fa = 1;
}


/* ---- F_703E (original code at 0x703E) ---- */
void ui_overlay_show(void)
{
    register int c;

    gb85++;
    c = f020f();
    switch (gb83) {
    case 1:
        gfx_color_select(gc0fc);
        f03a8(14, 184, 102, 12);
        f039f(14, 184, 102, 12);
        break;
    case 2:
        f03b4(24, 388, 148, 10, 24, 188);
        f039f(24, 188, 148, 10);
        break;
    case 3:
        gfx_color_select(0);
        f03a8(0, 188, 320, 12);
        f039f(0, 188, 320, 12);
        break;
    case 5:
        gfx_color_select(0);
        rect_border_draw(6, 162, 308, 36);
        gfx_color_select(gc0fc);
        f03a8(8, 163, 304, 34);
        f039f(6, 162, 308, 36);
        break;
    }
    gfx_color_select(c);
}


/* ---- F_7162 (original code at 0x7162) ---- */
/* gc0ee is declared as a near `char *` here (matching gc0f6/gc0f2) rather
   than the `char far *` used elsewhere: F_7162 only ever indexes through it
   with a near cast, and giving it a far type here would change the pushes
   the call below compiles to. Kept as a local exception. */
extern char *gc0ee, *gc0f6, *gc0f2;

void ui_overlay_hide(void)
{
    if (--gb85 < 0)
        gb85 = 0;
    else if (gb85)
        return;
    switch (gb83) {
    case 1:
        f03c9(14, 0xb8, gc0ee + ((unsigned *)gc0ee)[2] + 2);
        f039f(14, 0xb8, 0x66, 12);
        break;
    case 2:
        hud_prompt_continue_draw();
        break;
    case 3:
        hud_prompt_select_draw(gc0f6);
        break;
    case 5:
        hud_prompt_message_draw(gc0f2, gc0ec);
        break;
    }
}


/* ---- F_71FB (original code at 0x71FB) ---- */
/* F_71FB -- clear the counter at DS:0B85.  No frame (rule 11).  The pilot
   draft spelled this global `_g_flag`, a hand-chosen name the declared
   naming convention cannot decide; it is spelled by the convention here. */

void ui_overlay_reset()
{
    gb85 = 0;
}


/* ---- F_7202 (original code at 0x7202) ---- */
/* gc0ee is declared as a near `char *` here for the same reason as F_7162. */
extern char *gc0ee;

void hud_draw_meter()
{
    register int i, x;
    int n;

    x = 16;
    n = puzzle_piece_count() / 2;
    for (i = 0; i < n; i++) {
        f03c9(x, 0xb0, gc0ee + ((unsigned *)gc0ee)[1] + 2);
        x += 18;
    }
    if (gc0fa)
        f039f(16, 0xb0, 0x6a, 4);
}


/* ---- F_726A (original code at 0x726A) ---- */
/* F_726A -- reset the two scroll counters. */

void hud_scroll_reset(void)
{
    g0b7e = 0;
    gb80 = 4;
}


/* ---- F_7277 (original code at 0x7277) ---- */
/* F_7277 -- frameless accessor.  The EB 00 is the return's jump to the
   function exit, which is the next instruction. */

int hud_tab_get(void)
{
    return gb7e;
}


/* ---- F_727D (original code at 0x727D) ---- */
int hud_tab_next(void)
{
    if (++gb7e >= 3)
        gb7e = 0;
    hud_tab_draw();
    return gb7e;
}


/* ---- F_7298 (original code at 0x7298) ---- */
/* gc0ee is declared as a near `char *` here for the same reason as F_7162. */
extern char *gc0ee;

void hud_tab_draw()
{
    f03c9(0x98, 0xa6, gc0ee + ((unsigned *)(gc0ee + 6))[gb7e] + 2);
    if (gb7e == 2)
        f03c9(0xa6, 0xae, gc0ee + ((unsigned *)(gc0ee + 12))[gb80] + 2);
    if (gc0fa)
        f039f(0x98, 0xa6, 0x2c, 0x1c);
}


/* ---- F_7313 (original code at 0x7313) ---- */
/* F_7313 -- advance the 0..4 clamped counter gb80 by n and report it.
   The dead `jmp` at 732A is the skip-over-the-else jump TC 2.0 emits at the
   end of EVERY then-branch that has an else, even when the then-branch ends
   in a `return` and the jump is therefore unreachable.  The `< 0` test uses
   the flags left by `add [gb80],ax`, which is why the += is written inside
   the condition. */

int hud_scroll_move(n)
int n;
{
    if ((gb80 += n) < 0) {
        gb80 = 0;
        return -1;
    } else {
        if (gb80 > 4)
            gb80 = 4;
    }
    hud_tab_draw();
    return gb80;
}


/* ---- F_7343 (original code at 0x7343) ---- */
/* Callers pass a promoted int; the stored byte is the low half. */
void energy_set(int value) { energy_meter = value; }


/* ---- F_734E (original code at 0x734E) ---- */
/* F_734E -- step the signed char energy_meter by n, clamp it at 4, notify f738A,
   and return it.  The reload `mov al,[energy_meter]` before the clamp test is the
   VALUE of the assignment expression: `add [energy_meter],al` leaves the result in
   memory, not in a register, so writing the += inside the condition forces
   TC to read it back.  A separate `energy_meter += n; if (energy_meter > 4)` compares memory
   in place and loses the reload. */

int energy_adjust(n)
int n;
{
    if (n != 0) {
        if (energy_meter == 2 && n < 0)
            tutorial_hint_dialog_show(4);
        if ((energy_meter += n) > 4)
            energy_meter = 4;
        energy_draw();
    }
    return energy_meter;
}


/* ---- F_738A (original code at 0x738A) ---- */
void energy_draw()
{
    register int x, c;

    c = f020f();
    x = energy_meter << 4;
    if (display_mode == 2)
        gfx_color_select(3);
    else
        gfx_color_select(2);
    if (x > 0)
        f03a8(0xf4, 0xa4, x, 10);
    gfx_color_select(8);
    if (x < 64)
        f03a8(x + 0xf4, 0xa4, 64 - x, 10);
    if (gc0fa)
        f039f(0xf4, 0xa4, 64, 10);
    gfx_color_select(c);
}


/* ---- F_7417 (original code at 0x7417) ---- */
struct R7417 { char pad[22]; unsigned offsets[1]; };
extern char far *gc0ee;

void f7417(void) { f03c9(244,175,(char far *)gc0ee + ((struct R7417 far *)gc0ee)->offsets[tick_div8()] + 2); }


/* ---- F_7443 (original code at 0x7443) ---- */
struct R7443 { char pad[32]; unsigned offsets[1]; };
extern char far *gc0ee;

void f7443(void) { register int i; i=campaign_node_index(); f03c9(244+i*16,186,(char far *)gc0ee + ((struct R7443 far *)gc0ee)->offsets[i] + 2); }


/* ---- F_747B (original code at 0x747B) ---- */
/* F_747B -- repaint the whole HUD frame: save the current palette/mode word
   f020F returns, switch to 0, lay down the two horizontal rules, the four
   panel fills and the twelve icon blits through the thunks f03A2 / f03A5 /
   f03A8, then restore the saved word. */

/*@PUB _hud_frame_draw*/
void hud_frame_draw(void)
{
    register int save;

    save = f020f();
    gfx_color_select(0);
    f03a5(0, 0xd, 0xba);
    f03a5(0x13f, 0xd, 0xba);
    f03a2(0, 0xc7, 0x140);
    f03a2(6, 0xf, 0x134);
    f03a2(6, 0xa0, 0x134);
    f03a5(6, 0x10, 0x90);
    f03a5(7, 0x10, 0x90);
    f03a5(0x138, 0x10, 0x90);
    f03a5(0x139, 0x10, 0x90);
    f03a5(6, 0xa2, 0x24);
    f03a5(7, 0xa2, 0x24);
    f03a5(0x138, 0xa2, 0x24);
    f03a5(0x139, 0xa2, 0x24);
    gfx_color_select(9);
    f03a2(1, 0xd, 0x13e);
    f03a2(1, 0xe, 0x13e);
    f03a2(6, 0xa1, 0x134);
    f03a2(1, 0xc6, 0x13e);
    f03a8(1, 0xd, 5, 0xba);
    f03a8(0x13a, 0xd, 5, 0xba);
    gfx_color_select(save);
}
