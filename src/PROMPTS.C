#include "VIDEO.H"
/* src/PROMPTS.C: Prompt boxes: continue, select and confirm.
   One translation unit; the sections below were the separate member
   sources of grouped module C_75F3_7856 and keep their original ids. */

extern int cur_color_index_get(), sprite_sheet_index_get();
extern void sprite_sheet_select();
extern void gfx_color_select(int n);
extern void text_draw_wrapped(int, int, char far *);
extern void rect_border_draw();
extern void anim_step_loop();
extern int menu_list_active(), keyboard_read_blocking_hotkeys(), hud_prompt_confirm_draw();
extern void menu_list_disable(void), menu_list_enable(), hud_panel_open();
extern int hud_prompt_kind, gc0fc;
extern char *gc0f6;
extern char display_mode;                      /* DS:BFCD */

/* ---- F_75F3 (original code at 0x75F3) ---- */
/* The local array initializer emits the original SCOPY argument order and
   the module-owned 15-byte _DATA contribution. */

/* Shared UI state immediately precedes the caption initializers.
   Consumers establish widths: F_7313 int, F_734E char, F_703E/F_7162 ints.
   The final zero byte is part of gb85, not alignment padding. */
int gb80 = 4;
char energy_meter = 4;
int hud_prompt_kind = 0;
int gb85 = 0;

void hud_prompt_continue_draw()
{
    char cap[15] = "\027\030 to Continue";                     /* bp-10 */
    register int s, d;                  /* si, di */

    hud_prompt_kind = 2;
    s = cur_color_index_get();
    d = sprite_sheet_index_get();
    if (display_mode == 2)
        gfx_color_select(5);
    else
        gfx_color_select(0xf);
    sprite_sheet_select(0);
    text_draw_wrapped(0x18, 0xbc, &cap);
    gfx_box(0x18, 0xbc, 0x94, 0xa);
    gfx_color_select(s);
    sprite_sheet_select(d);
}


/* ---- F_7676 (original code at 0x7676) ---- */
void hud_prompt_continue_clear(void) { anim_step_loop(0x18, 0x184, 0x94, 10, 0x18, 0xbc); }


/* ---- F_7695 (original code at 0x7695) ---- */
hud_prompt_select_draw(p)
char *p;
{
    char cap[13] = "\027\030 to Select";
    register int s, d;

    s = cur_color_index_get();
    d = sprite_sheet_index_get();
    hud_prompt_kind = 3;
    gc0f6 = p;
    gc0fc = 0;
    gfx_color_select(0);
    gfx_clear_rect(0, 0xbc, 0x140, 12);
    gfx_color_select(15);
    sprite_sheet_select(0);
    text_draw_wrapped(10, 0xbd, p);
    text_draw_wrapped(0xaa, 0xbd, &cap);
    gfx_box(0, 0xbc, 0x140, 12);
    gfx_color_select(s);
    sprite_sheet_select(d);
}


/* ---- F_7747 (original code at 0x7747) ---- */
void hud_prompt_message_run(char far *p) { register int key, saved; saved=menu_list_active(); menu_list_disable(); hud_prompt_confirm_draw(p,0,15,1,0); do { key=keyboard_read_blocking_hotkeys(); } while (key!=13 && key!=27); if(saved) menu_list_enable(); hud_panel_open(); }


/* ---- F_778B (original code at 0x778B) ---- */
hud_prompt_confirm_draw(p,a,b,c,e)
char *p;
int a,b,c,e;
{
    char cap[15] = "\027\030 to Continue";
    register int s, d;

    s = cur_color_index_get();
    d = sprite_sheet_index_get();
    hud_prompt_kind = 4;
    gc0fc = b;
    gfx_color_select(0);
    rect_border_draw(6, 0xa2, 0x134, 0x24);
    gfx_color_select(b);
    gfx_clear_rect(8, 0xa3, 0x130, 0x22);
    gfx_color_select(a);
    sprite_sheet_select(c);
    text_draw_wrapped(12, 0xa5, p);
    if (e)
        text_draw_wrapped(0xaa, 0xb9, &cap);
    gfx_box(6, 0xa2, 0x134, 0x24);
    gfx_color_select(s);
    sprite_sheet_select(d);
}
