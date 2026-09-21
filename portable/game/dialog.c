/* dialog.c -- portable port of src/DIALOG.C: dialog boxes (layout,
 * drawing, shadow save/restore, key loop).
 * One translation unit; the sections below were the separate member
 * sources of grouped module C_7D91_880A and keep their original ids.
 *
 * `struct input` (dialog_list_run's parameter) is already defined by
 * game_funcs.h (it is referenced there for that prototype) -- not
 * re-declared here per tu-porting-rules.md ("Struct tags come from
 * game_structs.h only; never re-declare a struct", extended to the
 * locally-defined tags game_funcs.h itself carries for the same reason).
 */
#include "game.h"

/* ---- F_7D91 (original code at 0x7D91) ---- */
dos_int dialog_draw_panel(struct gc0fe_record *p)
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
    return 0;   /* PORT: original had no return statement (K&R implicit int) */
}

/* ---- F_7DF1 (original code at 0x7DF1) ---- */
void menu_list_source_set_default(void)
{
    menu_list_source_set(g0d36);
}

/* ---- F_7DFC (original code at 0x7DFC) ---- */
void menu_list_source_set_players(void)
{
    menu_list_source_set(g0d78);
}

/* ---- F_7E07 (original code at 0x7E07) ----
 * PORT: dialog_button1_label_off/dialog_button2_label_off historically held
 * raw DS near-pointer constants (0x0D9A/0x0DA9/0x0DB7/0x0DBC/0x0DC0)
 * into the button-caption strings; cross-checked against
 * recipes/data/*.json, these are exactly DATA_0109CA_BACK_PAGE ("Go Back a
 * Page"), DATA_0109D9_NEXT_PAGE ("See Next Page"), DATA_0109E7_DONE
 * ("Done"), DATA_0109EC_YES ("Yes") and DATA_0109F0_NO ("No") -- all
 * already available as named generated objects.  Paragraph/near-pointer
 * arithmetic never survives the port (tu-porting-rules.md sec 6); the
 * named objects are used directly instead of the raw offsets. */
void dialog_layout(struct dialog *p)
{
    dos_int w1;
    dos_int nlines;
    dos_char *q;
    dos_int s, d;

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
        dialog_button_str = 0x0d7e;
        s = 0x78;
        break;
    case 2:
        dialog_box_h += 13;
        dialog_button_str = 0x0d8b;
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
        dialog_button1_label_off = (dos_char *)DATA_0109D9_NEXT_PAGE;
        dialog_button1_label_w = 0x78;
        dialog_button1_label_rows = 0xd;
        s = 0x8c;
        dialog_box_h += dialog_button1_label_rows + 3;
        break;
    case 5:
        dialog_text_inset_x2 = 6;
        dialog_text_inset_y = 4;
        dialog_text_inset_x = 0xb;
        dialog_button1_label_off = (dos_char *)DATA_0109CA_BACK_PAGE;
        dialog_button2_label_off = (dos_char *)DATA_0109D9_NEXT_PAGE;
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
        dialog_button1_label_off = (dos_char *)DATA_0109CA_BACK_PAGE;
        dialog_button2_label_off = (dos_char *)DATA_0109E7_DONE;
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
        dialog_button1_label_off = (dos_char *)DATA_0109F0_NO;
        dialog_button2_label_off = (dos_char *)DATA_0109EC_YES;
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
        dialog_button2_label_cx = (dialog_button2_label_w - text_line_width(dialog_button2_label_off)) / 2 + dialog_button2_label_x;
        /* fallthrough */
    case 4:
        dialog_button1_label_cx = (dialog_button1_label_w - text_line_width(dialog_button1_label_off)) / 2 + dialog_button1_label_x;
        break;
    }
}

/* ---- F_8267 (original code at 0x8267) ----
 * PORT: the historical code reads dialog_button1_label_x/dialog_divider1_y/
 * dialog_button1_label_w/dialog_button1_label_rows/dialog_button1_label_cx/
 * dialog_button1_label_off through a same-address array cast over the
 * button-1/button-2 word pairs, relying on the historical DS segment
 * placing each pair at adjacent addresses.  portable/generated/
 * game_state.h declares each of these as an independent global (not an
 * array), so C gives no adjacency guarantee for them; a per-index cast
 * would be undefined behaviour here.  Replaced with an explicit n==0/n==1
 * selector below -- every call site passes 0 or 1, so this is a pure
 * indexing-mechanism change, not a semantics change. */
static dos_int dlg_x(dos_int n)     { return n ? dialog_button2_label_x : dialog_button1_label_x; }
static dos_int dlg_y(dos_int n)     { return n ? dialog_divider2_y : dialog_divider1_y; }
static dos_int dlg_w(dos_int n)     { return n ? dialog_button2_label_w : dialog_button1_label_w; }
static dos_int dlg_rows(dos_int n)  { return n ? dialog_button2_label_rows : dialog_button1_label_rows; }
static dos_int dlg_cx(dos_int n)    { return n ? dialog_button2_label_cx : dialog_button1_label_cx; }
static dos_char *dlg_off(dos_int n) { return n ? dialog_button2_label_off : dialog_button1_label_off; }

void dialog_draw_button(dos_int n)
{
    dos_int u, v;
    dos_int y, x;

    x = dlg_x(n);
    y = dlg_y(n);
    u = dlg_w(n);
    v = dlg_rows(n);
    gfx_bar(x + 1, y, u - 2);
    gfx_bar(x + 1, y + v - 1, u - 2);
    gfx_vline(x, y + 1, v - 2);
    gfx_vline(x + u - 1, y + 1, v - 2);
    gfx_bar(x + 1, y + 1, 1);
    gfx_bar(x + 1, y + v - 2, 1);
    gfx_bar(x + u - 2, y + 1, 1);
    gfx_bar(x + u - 2, y + v - 2, 1);
    text_draw_wrapped(dlg_cx(n), (v - 10) / 2 + y + 1, dlg_off(n));
}

/* ---- F_8378 (original code at 0x8378) ---- */
void dialog_fill_box(dos_int n)
{
    dos_int u, v;
    dos_int x, y;

    x = dlg_x(n);
    y = dlg_y(n);
    u = dlg_w(n);
    v = dlg_rows(n);
    gfx_fill_rect(x + 1, y + 2, u - 2, v - 4);
    gfx_fill_rect(x + 2, y + 1, u - 4, 1);
    gfx_fill_rect(x + 2, y + v - 2, u - 4, 1);
    gfx_box(x, y, u, v);
}

/* ---- F_8414 (original code at 0x8414) ---- */
void dialog_blink_box(dos_int a)
{
    dos_int i;

    for (i = 0; i < 4; i++) {
        dialog_fill_box(a);
        timer_wait_ticks(14);
    }
}

/* ---- F_8434 (original code at 0x8434) ----
 * PORT: the historical two-int (ui_gfx_blob/dialog_backdrop_save_size)
 * argument pair was the offset/segment halves of ui_gfx_blob's far
 * pointer, pushed separately for byte-exactness under the -mc calling
 * convention (see the file's own top-of-file comment).  gfx.h's
 * gfx_save_rect already takes a real pointer; the segment half
 * (dialog_backdrop_save_size, a DS-adjacency alias with no portable
 * object per docs/portable/state-map.md's "Segment-half aliases") is
 * retired along with the paragraph arithmetic (tu-porting-rules.md sec 6). */
void dialog_draw_shadow(void)
{
    gfx_save_rect(dialog_box_x, dialog_box_y, dialog_box_w, dialog_box_h, ui_gfx_blob);
}

/* ---- F_8453 (original code at 0x8453) ---- */
void dialog_restore_screen(void)
{
    gfx_restore_rect(dialog_box_x, dialog_box_y, ui_gfx_blob);
    gfx_box(dialog_box_x, dialog_box_y, dialog_box_w, dialog_box_h);
}

/* ---- F_8480 (original code at 0x8480) ----
 * PORT: dialog_button_str historically held a raw DS near-pointer constant
 * (0x0D7E or 0x0D8B) into a button-caption string that, by a linker-layout
 * coincidence, the source's own comments do not spell out in full: 0x0D7E
 * is the LAST byte of DATA_010924_MENU_DESCRIPTORS (value 0x1F,
 * recipes/data/DATA_010924_MENU_DESCRIPTORS.json's terminal_control field)
 * immediately followed by DATA_0109AF_GO_BACK (" to Go Back"); 0x0D8B is
 * DATA_0109BB_CONTROL_CODES ({0x17,0x18}) immediately followed by
 * DATA_0109BD_CONTINUE (" to Continue") -- exactly the "\027\030 to
 * Continue" caption src/PROMPTS.C spells as a literal elsewhere.  Cross-
 * checked against recipes/data/*.json.  Paragraph/near-pointer arithmetic
 * never survives the port (tu-porting-rules.md sec 6); the two literal
 * byte sequences are reproduced below and selected by the same historical
 * sentinel value dialog_button_str still holds. */
static const dos_char dialog_button_str_go_back[] =
    { 0x1F, ' ', 't', 'o', ' ', 'G', 'o', ' ', 'B', 'a', 'c', 'k', 0 };
static const dos_char dialog_button_str_continue[] =
    { 0x17, 0x18, ' ', 't', 'o', ' ', 'C', 'o', 'n', 't', 'i', 'n', 'u', 'e', 0 };

static dos_char *dialog_button_str_text(void)
{
    switch (dialog_button_str) {
    case 0x0d7e: return (dos_char *)dialog_button_str_go_back;
    case 0x0d8b: return (dos_char *)dialog_button_str_continue;
    default:     return 0;
    }
}

void dialog_draw(struct dialog *p, dos_int first)
{
    dos_int a, b, x, w, h;
    dos_int y, i;

    a = cur_color_index_get();
    b = sprite_sheet_index_get();
    sprite_sheet_select(0);
    dialog_layout(p);
    if (first) dialog_draw_shadow();
    gfx_color_select(15);
    gfx_clear_rect(dialog_box_x, dialog_box_y, dialog_box_w, dialog_box_h);
    x = dialog_box_x + 4;
    y = dialog_box_y + 2;
    if (p->kind != 2) y += 2;
    w = dialog_box_w - 10;
    h = dialog_box_h - 8;
    if (p->kind != 2) h -= 2;
    gfx_color_select(0);
    for (i = 0; i < 2; i++) { w += 2; h += 2; rect_border_draw(--x, --y, w, h); }
    gfx_color_select(0);
    for (i = 0; i < 2; i++) gfx_vline(x + w + i, y + 2, h);
    for (i = 0; i < 2; i++) gfx_bar(x + 2, y + h + i, w);
    x += 2; y += 2; w -= 4;
    if (p->title) {
        y += 2;
        gfx_color_select(0);
        text_draw_wrapped(dialog_title_x, y, p->title);
        y += 11;
        gfx_color_select(0);
        gfx_bar(x, y, w);
    }
    y += dialog_text_inset_y;
    gfx_color_select(0);
    text_draw_wrapped(x + dialog_text_inset_x2, y, p->text);
    if (dialog_button_str) {
        gfx_color_select(0);
        gfx_bar(x, dialog_button_label_y - 2, w);
        gfx_color_select(0);
        text_draw_wrapped(dialog_button_label_x, dialog_button_label_y, dialog_button_str_text());
    }
    if (p->kind == 4 || p->kind == 5 || p->kind == 6) {
        gfx_color_select(0);
        gfx_bar(x, dialog_divider1_y - 2, w);
    }
    switch (p->kind) {
    case 5: case 6: case 7: dialog_draw_button(1); /* fallthrough */
    case 4: dialog_draw_button(0); dialog_fill_box(p->initial); break;
    }
    gfx_box(dialog_box_x, dialog_box_y, dialog_box_w, dialog_box_h);
    gfx_color_select(a);
    sprite_sheet_select(b);
}

/* ---- F_86C9 (original code at 0x86C9) ---- */
dos_int dialog_run(struct dialog *p)
{
    dos_int a, b, c;
    dos_int i, n;

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
dos_int dialog_list_run(struct input *p)
{
    struct dialog v;
    dos_int outer, state, first, done, key;
    dos_int selected, direction;

    selected = 0; first = 1; direction = 0;
    outer = keyboard_chain_active(); keyboard_chain_enable(); keyboard_buffer_drain(); ui_overlay_show(); sound_start();
    state = menu_list_active(); menu_list_disable();
    v.title = p->title; v.sub = p->flag;
    v.cx = p->a; v.cy = p->b; v.w = p->c; v.lines = p->d;
    while (selected >= 0 && selected < p->count) {
        if (selected == 0) { v.kind = 4; direction = v.initial = 0; }
        else { if (selected < p->count - 1) v.kind = 5; else v.kind = 6; direction = v.initial = 1; }
        v.text = p->records[selected];
        dialog_draw(&v, first); first = 0; done = 0;
        while (!done) {
            key = keyboard_read_blocking_hotkeys();
            switch (key) {
            case 9: case 0x148: case 0x14b: case 0x14d: case 0x150:
                if (selected > 0) { dialog_fill_box(direction); direction = !direction; dialog_fill_box(direction); }
                break;
            case 13: dialog_blink_box(direction); done = 1; break;
            case 27: direction = -1; done = 1; break;
            case 0x13b: direction = 2; done = 1; break;
            case 0x149:
                if (selected > 0) { if (direction > 0) dialog_fill_box(direction); direction = 0; dialog_blink_box(0); done = 1; }
                break;
            case 0x151:
                if (selected == 0) { direction = 0; dialog_blink_box(0); done = 1; }
                else if (selected < p->count - 1) { direction = 1; dialog_blink_box(1); done = 1; }
                break;
            }
        }
        switch (direction) {
        case -1: selected = -1; break;
        case 0: if (selected == 0) ++selected; else --selected; break;
        case 1: ++selected; break;
        case 2: selected = p->count; break;
        }
    }
    dialog_restore_screen(); if (state) menu_list_enable(); sound_request_count_dec(); ui_overlay_hide(); if (!outer) keyboard_chain_disable(); keyboard_buffer_drain();
    if (selected < 0) return 0; else return 1;
}
