/* src/DIALOG.C: Dialog boxes: layout, drawing, shadow save/restore and the key loop.
   One translation unit; the sections below were the separate member
   sources of grouped module C_7D91_880A and keep their original ids. */

#include "DIALOG.H"
#include "GC0FE.H"
#include "LAYOUT.H"
#include "VIDEO.H"

/* Symbols redeclared identically (same form) by two or more sections are
   merged here. Symbols that a section needs in a different byte-significant
   form (a prototype vs. K&R empty parens, or a narrower/wider type) are kept
   local to that section instead -- e.g. F_7DF1's prototyped `menu_list_source_set(char
   far *)` versus F_7DFC's K&R `menu_list_source_set()`, F_8267's K&R `text_draw_wrapped()`
   versus F_8480's prototyped form, and F_8434/F_8453's `unsigned
   ui_gfx_blob`/`dialog_backdrop_save_size` (documented in LAYOUT.H: those two thunks push the
   far pointer's two words separately, so a `char far *` type here would add
   an extra push and break byte-exactness). */
extern void gfx_vline();
extern int keyboard_chain_active(), menu_list_active(), keyboard_read_blocking_hotkeys();
extern void keyboard_chain_enable(void);
extern void sound_start(void);
extern void menu_list_disable(void);
extern void keyboard_buffer_drain(void);
extern void dialog_fill_box();
extern void ui_overlay_hide(void);
extern void ui_overlay_show(void);
extern void menu_list_enable(void);
extern void dialog_restore_screen();
extern void sound_request_count_dec(void);
extern void keyboard_chain_disable(void);
extern void dialog_blink_box(int);
extern unsigned ui_gfx_blob;                  /* DS:C5CA */
extern unsigned dialog_backdrop_save_size;                  /* DS:C5CC */

/* ---- F_7D91 (original code at 0x7D91) ---- */
dialog_draw_panel(p)
    struct gc0fe_record far *p;
{
    struct dialog q;

    q.kind = 2;
    q.text = p->text;
    q.title = 0;
    q.sub = 1;
    q.initial = 0;
    q.cx = p->x;
    q.cy = 13;
    q.w = p->width;
    q.lines = p->count;
    dialog_draw(&q, 1);
}


/* ---- F_7DF1 (original code at 0x7DF1) ---- */
/* F_7DF1 -- hand one DGROUP buffer to menu_list_source_set.  Compact model: the array name
   becomes a far pointer, push ds / mov ax,OFFSET / push ax. */
extern char g0d36[];
extern void menu_list_source_set(char far *);

void menu_list_source_set_default(void)
{
    menu_list_source_set(g0d36);
}


/* ---- F_7DFC (original code at 0x7DFC) ---- */
extern void menu_list_source_set();
extern char g0d78[];
void menu_list_source_set_players(void) { menu_list_source_set(g0d78); }


/* ---- F_7E07 (original code at 0x7E07) ---- */
/* F_7E07 -- lay out a dialog box: measure the text, pick the geometry for the
   box kind from three switches, and leave the result in the C1xx block. */
extern int text_line_width();

void dialog_layout(struct dialog far *p)
{
    int w1;                             /* bp-8 */
    int nlines;                         /* bp-6 */
    char far *q;                        /* bp-4 */
    register int s, d;                  /* si, di */

    d = 0;
    s = 0;
    if ((nlines = p->lines) == -1) {
        nlines = 1;
        q = p->text;
        while (*q != 0) {
            if (*q == 0xa || *q == 0xd) nlines++;
            q++;
        }
    }
    dialog_box_h = nlines * 10 + 10;
    if (p->kind == 2) dialog_box_h -= 2;
    if (p->title == 0) {
        dialog_box_w = w1 = 0;
    } else {
        dialog_box_h += 14;
        dialog_box_w = w1 = text_line_width(p->title);
    }
    switch (p->sub) {
    case 0:
        dialog_button_str = 0;
        break;
    case 1:
        dialog_box_h += 13;
        dialog_button_str = 0xd7e;
        s = 0x78;
        break;
    case 2:
        dialog_box_h += 13;
        dialog_button_str = 0xd8b;
        s = 0x94;
        break;
    }
    switch (p->kind) {
    case 0:
    case 1:
        if (p->title == 0) {
            dialog_text_inset_x2 = 4;
            dialog_text_inset_y = 6;
            dialog_text_inset_x = 7;
        } else {
            dialog_text_inset_x2 = 8;
            dialog_text_inset_y = 4;
            dialog_text_inset_x = 9;
        }
        break;
    case 2:
    case 3:
        dialog_text_inset_x2 = 0;
        dialog_text_inset_y = 2;
        dialog_text_inset_x = 4;
        break;
    case 4:
        dialog_text_inset_x2 = 6;
        dialog_text_inset_y = 4;
        dialog_text_inset_x = 0xb;
        dialog_button1_label_off = (char near *)0xda9;
        dialog_button1_label_w = 0x78;
        dialog_button1_label_rows = 0xd;
        s = 0x8c;
        dialog_box_h += dialog_button1_label_rows + 3;
        break;
    case 5:
        dialog_text_inset_x2 = 6;
        dialog_text_inset_y = 4;
        dialog_text_inset_x = 0xb;
        dialog_button1_label_off = (char near *)0xd9a;
        dialog_button2_label_off = (char near *)0xda9;
        dialog_button1_label_w = 0x78;
        dialog_button2_label_w = 0x78;
        dialog_button1_label_rows = 0xd;
        dialog_button2_label_rows = 0xd;
        s = 0x10e;
        dialog_box_h += dialog_button1_label_rows + 3;
        break;
    case 6:
        dialog_text_inset_x2 = 6;
        dialog_text_inset_y = 4;
        dialog_text_inset_x = 0xb;
        dialog_button1_label_off = (char near *)0xd9a;
        dialog_button2_label_off = (char near *)0xdb7;
        dialog_button1_label_w = 0x78;
        dialog_button2_label_w = 0x78;
        dialog_button1_label_rows = 0xd;
        dialog_button2_label_rows = 0xd;
        s = 0x10e;
        dialog_box_h += dialog_button1_label_rows + 3;
        break;
    case 7:
        dialog_text_inset_x2 = 8;
        dialog_text_inset_y = 4;
        dialog_text_inset_x = 4;
        dialog_button1_label_off = (char near *)0xdc0;
        dialog_button2_label_off = (char near *)0xdbc;
        dialog_button1_label_w = 0x28;
        dialog_button2_label_w = 0x28;
        dialog_button1_label_rows = 0xb;
        dialog_button2_label_rows = 0xb;
        s = 0x6e;
        dialog_box_h += dialog_button1_label_rows + 8;
        break;
    }
    if (dialog_box_w < s) dialog_box_w = s;
    dialog_box_h += dialog_text_inset_y + dialog_text_inset_x;
    if ((d = p->w) == -1) {
        d = text_line_width(p->text);
        d += dialog_text_inset_x2 * 2;
        d += 2;
    }
    if (dialog_box_w < d) dialog_box_w = d;
    dialog_box_w += 10;
    if ((dialog_box_x = p->cx) == -1) dialog_box_x = (0x140 - dialog_box_w) / 2;
    if ((dialog_box_y = p->cy) == -1) dialog_box_y = (0xc8 - dialog_box_h) / 2;
    dialog_title_x = (dialog_box_w - 2 - w1) / 2 + dialog_box_x;
    dialog_button_label_x = (dialog_box_w - 2 - s) / 2 + dialog_box_x;
    switch (p->kind) {
    case 0:
    case 1:
    case 2:
    case 3:
        dialog_button_label_y = dialog_box_y + dialog_box_h - 0x11;
        break;
    case 4:
        dialog_button1_label_x = dialog_box_x + dialog_box_w - 0x88;
        dialog_divider1_y = dialog_box_y + dialog_box_h - (dialog_button1_label_rows + 7);
        break;
    case 5:
        dialog_button1_label_x = dialog_box_x + 14;
        dialog_button2_label_x = dialog_box_x + dialog_box_w - 0x88;
        dialog_divider1_y = dialog_divider2_y = dialog_box_y + dialog_box_h - (dialog_button1_label_rows + 7);
        break;
    case 6:
        dialog_button1_label_x = dialog_box_x + 14;
        dialog_button2_label_x = dialog_box_x + dialog_box_w - 0x88;
        dialog_divider1_y = dialog_divider2_y = dialog_box_y + dialog_box_h - (dialog_button1_label_rows + 7);
        break;
    case 7:
        dialog_button_label_y = dialog_box_y + dialog_box_h - 0x11;
        dialog_button1_label_x = dialog_box_x + dialog_box_w - 0x38;
        dialog_button2_label_x = dialog_button1_label_x - 0x32;
        dialog_divider1_y = dialog_divider2_y = dialog_box_y + dialog_box_h - (dialog_button1_label_rows + 0x17);
        break;
    }
    switch (p->kind) {
    case 5:
    case 6:
    case 7:
        dialog_button2_label_cx = (dialog_button2_label_w - text_line_width((char far *)dialog_button2_label_off)) / 2 + dialog_button2_label_x;
    case 4:
        dialog_button1_label_cx = (dialog_button1_label_w - text_line_width((char far *)dialog_button1_label_off)) / 2 + dialog_button1_label_x;
        break;
    }
}


/* ---- F_8267 (original code at 0x8267) ---- */
/* F_8267 -- draw one box's border: eight edge/corner blits through the
   thunks at IP 03A2h and 03A5h, then the caption.  The same four parallel
   int arrays F_8378 uses, each reached as ONE near DS displacement and
   recomputed for every access (rules 2 and 6).  SI and DI go by DECLARATION
   order, not by first use (rule 22): here the DS:C10C value is declared
   first and takes SI, and the DS:C11C value takes DI -- the opposite
   assignment to F_8378, from the opposite declaration order.  The caption's
   third argument is a NEAR pointer widened by a cast, so the segment is
   pushed as plain `ds` (rule 20's data form). */
extern void text_draw_wrapped();

void dialog_draw_button(n)
int n;
{
    /* dialog_button1_label_x/dialog_divider1_y/dialog_button1_label_w/dialog_button1_label_rows/dialog_button1_label_cx/dialog_button1_label_off are scalar DS:C1xx words per
       LAYOUT.H (shared file-scope declaration with this module's other
       members); this function indexes the same addresses with a variable
       subscript n, so each is read through a same-address array cast
       rather than its own conflicting extern array declaration. */
    int u, v;
    register int y, x;

    x = ((int *)&dialog_button1_label_x)[n];
    y = ((int *)&dialog_divider1_y)[n];
    u = ((int *)&dialog_button1_label_w)[n];
    v = ((int *)&dialog_button1_label_rows)[n];
    gfx_bar(x + 1, y, u - 2);
    gfx_bar(x + 1, y + v - 1, u - 2);
    gfx_vline(x, y + 1, v - 2);
    gfx_vline(x + u - 1, y + 1, v - 2);
    gfx_bar(x + 1, y + 1, 1);
    gfx_bar(x + 1, y + v - 2, 1);
    gfx_bar(x + u - 2, y + 1, 1);
    gfx_bar(x + u - 2, y + v - 2, 1);
    text_draw_wrapped(((int *)&dialog_button1_label_cx)[n], (v - 10) / 2 + y + 1, (char far *)((char near **)&dialog_button1_label_off)[n]);
}


/* ---- F_8378 (original code at 0x8378) ---- */
/* F_8378 -- draw one framed box: three edge blits through the thunk at IP
   03ABh and one fill through 039Fh.  Four parallel int arrays indexed by the
   same parameter, each reached as ONE near DS displacement (rule 2, scalar
   array element); TC 2.0 performs no CSE, so `mov bx,[bp+4] / shl bx,1`
   is recomputed for every one of them (rule 6).  Two register locals take
   SI then DI in declaration order (rule 12) and the two stack locals lie in
   reverse declaration order upward from bp (rule 1). */

void dialog_fill_box(n)
int n;
{
    /* See F_8267.C: dialog_button1_label_x/dialog_divider1_y/dialog_button1_label_w/dialog_button1_label_rows are scalar DS:C1xx words per
       LAYOUT.H (shared file-scope declaration with this module's other
       members); indexed here through a same-address array cast instead of
       a conflicting extern array declaration. */
    int u, v;
    register int x, y;

    x = ((int *)&dialog_button1_label_x)[n];
    y = ((int *)&dialog_divider1_y)[n];
    u = ((int *)&dialog_button1_label_w)[n];
    v = ((int *)&dialog_button1_label_rows)[n];
    gfx_fill_rect(x + 1, y + 2, u - 2, v - 4);
    gfx_fill_rect(x + 2, y + 1, u - 4, 1);
    gfx_fill_rect(x + 2, y + v - 2, u - 4, 1);
    gfx_box(x, y, u, v);
}


/* ---- F_8414 (original code at 0x8414) ---- */
/* F_8414 -- draw the parameter four times, one tick apart.  A plain
   parameter with a `register` local gives the LOCAL si (rule 12), and there
   is no `sub sp` because the only local IS the register variable. */
extern void timer_wait_ticks();

void dialog_blink_box(a)
int a;
{
    register int i;

    for (i = 0; i < 4; i++) {
        dialog_fill_box(a);
        timer_wait_ticks(14);
    }
}


/* ---- F_8434 (original code at 0x8434) ---- */
/* F_8434 -- one blit through the runtime-generated thunk at IP 03AEh, six
   DGROUP words wide.  Two adjacent words pushed segment-then-offset and two
   plain ints are the same six pushes, so they are spelled as the six ints
   the extent literally pushes. */
extern void gfx_save_rect();

void dialog_draw_shadow()
{
    gfx_save_rect(dialog_box_x, dialog_box_y, dialog_box_w, dialog_box_h, ui_gfx_blob, dialog_backdrop_save_size);
}


/* ---- F_8453 (original code at 0x8453) ---- */
/* F_8453 -- the pair of blits through the runtime-generated thunks at IP
   03B1h and 039Fh. */
extern void gfx_restore_rect();

void dialog_restore_screen()
{
    gfx_restore_rect(dialog_box_x, dialog_box_y, ui_gfx_blob, dialog_backdrop_save_size);
    gfx_box(dialog_box_x, dialog_box_y, dialog_box_w, dialog_box_h);
}


/* ---- F_8480 (original code at 0x8480) ---- */
extern int cur_color_index_get(), sprite_sheet_index_get();
extern void sprite_sheet_select();
extern void rect_border_draw();
extern void dialog_draw_shadow(void);
extern void gfx_color_select(int n);
extern void text_draw_wrapped(int, int, char far *);

void dialog_draw(struct dialog far *p, int first)
{int a,b,x,w,h;register int y,i;
a=cur_color_index_get();b=sprite_sheet_index_get();sprite_sheet_select(0);dialog_layout(p);if(first)dialog_draw_shadow();
gfx_color_select(15);gfx_clear_rect(dialog_box_x,dialog_box_y,dialog_box_w,dialog_box_h);
x=dialog_box_x+4;y=dialog_box_y+2;if(p->kind!=2)y+=2;w=dialog_box_w-10;h=dialog_box_h-8;if(p->kind!=2)h-=2;
gfx_color_select(0);for(i=0;i<2;i++){w+=2;h+=2;rect_border_draw(--x,--y,w,h);}
gfx_color_select(0);for(i=0;i<2;i++)gfx_vline(x+w+i,y+2,h);
for(i=0;i<2;i++)gfx_bar(x+2,y+h+i,w);
x+=2;y+=2;w-=4;
if(p->title){y+=2;gfx_color_select(0);text_draw_wrapped(dialog_title_x,y,p->title);y+=11;gfx_color_select(0);gfx_bar(x,y,w);}
y+=dialog_text_inset_y;gfx_color_select(0);text_draw_wrapped(x+dialog_text_inset_x2,y,p->text);
if(dialog_button_str){gfx_color_select(0);gfx_bar(x,dialog_button_label_y-2,w);gfx_color_select(0);text_draw_wrapped(dialog_button_label_x,dialog_button_label_y,(char *)(char near *)dialog_button_str);}
if(p->kind==4||p->kind==5||p->kind==6){gfx_color_select(0);gfx_bar(x,dialog_divider1_y-2,w);}
switch(p->kind){case 5:case 6:case 7:dialog_draw_button(1);case 4:dialog_draw_button(0);dialog_fill_box(p->initial);break;}
gfx_box(dialog_box_x,dialog_box_y,dialog_box_w,dialog_box_h);gfx_color_select(a);sprite_sheet_select(b);
}


/* ---- F_86C9 (original code at 0x86C9) ---- */
int dialog_run(struct dialog far *p)
{
    int a, b, c;
    register int i, n;

    n = 0;
    a = keyboard_chain_active();
    keyboard_chain_enable();
    ui_overlay_show();
    sound_start();
    b = menu_list_active();
    menu_list_disable();
    keyboard_buffer_drain();
    i = p->initial;
    dialog_draw(p, 1);
    while (!n) {
        c = keyboard_read_blocking_hotkeys();
        switch (p->kind) {
        case 1:
            switch (c) {
            case 13:
            case 27:
                n = 1;
                break;
            }
            break;
        case 7:
            switch (c) {
            case 78:
            case 110:
                if (i == 1) dialog_fill_box(i);
                i = 0;
                dialog_blink_box(0);
                n = 1;
                break;
            case 89:
            case 121:
                if (i == 0) dialog_fill_box(i);
                i = 1;
                dialog_blink_box(1);
                n = 1;
                break;
            case 9:
            case 328:
            case 331:
            case 333:
            case 336:
                dialog_fill_box(i);
                i = !i;
                dialog_fill_box(i);
                break;
            case 13:
                dialog_blink_box(i);
                n = 1;
                break;
            case 27:
                i = -1;
                n = 1;
                break;
            }
            break;
        }
    }
    dialog_restore_screen();
    if (b) menu_list_enable();
    sound_request_count_dec();
    ui_overlay_hide();
    if (!a) keyboard_chain_disable();
    keyboard_buffer_drain();
    return i;
}


/* ---- F_880A (original code at 0x880A) ---- */
/* Exact Turbo C reconstruction of the selection loop and its shared-frame
 * cleanup (formerly F_8C04). Packed layouts and local declaration order
 * reproduce the original 30-byte frame. No synthetic cleanup entry. */
struct input { char far *title; char flag; char far * far *records; signed char count; int a,b,c,d; };

int dialog_list_run(struct input far *p)
{
 struct dialog v;
 int outer,state,first,done,key;
 register int selected,direction;
 selected=0; first=1; direction=0;
 outer=keyboard_chain_active(); keyboard_chain_enable(); keyboard_buffer_drain(); ui_overlay_show(); sound_start();
 state=menu_list_active(); menu_list_disable();
 v.title=p->title; v.sub=p->flag;
 v.cx=p->a;v.cy=p->b;v.w=p->c;v.lines=p->d;
 while(selected>=0 && selected<p->count) {
  if(selected==0) { v.kind=4; direction=v.initial=0; }
  else { if(selected<p->count-1) v.kind=5; else v.kind=6; direction=v.initial=1; }
  v.text=p->records[selected];
  dialog_draw(&v,first); first=0;done=0;
  while(!done) {
   key=keyboard_read_blocking_hotkeys();
   switch(key) {
    case 9: case 0x148: case 0x14b: case 0x14d: case 0x150:
     if(selected>0) { dialog_fill_box(direction);direction=!direction;dialog_fill_box(direction); } break;
    case 13: dialog_blink_box(direction);done=1;break;
    case 27: direction=-1;done=1;break;
    case 0x13b: direction=2;done=1;break;
    case 0x149:
     if(selected>0) { if(direction>0) dialog_fill_box(direction);direction=0;dialog_blink_box(0);done=1; } break;
    case 0x151:
     if(selected==0) { direction=0;dialog_blink_box(0);done=1; }
     else if(selected<p->count-1) {direction=1;dialog_blink_box(1);done=1;} break;
   }
  }
  switch(direction) {
   case -1:selected=-1;break;
   case 0:if(selected==0) ++selected;else --selected;break;
   case 1:++selected;break;
   case 2:selected=p->count;break;
  }
 }
 dialog_restore_screen();if(state) menu_list_enable();sound_request_count_dec();ui_overlay_hide();if(!outer) keyboard_chain_disable();keyboard_buffer_drain();
 if(selected<0) return 0; else return 1;
}
