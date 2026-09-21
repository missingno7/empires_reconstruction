/* src/BOARDSCR.C: Board scrolling, unit scripts and animation.
   One translation unit; the sections below were the separate member
   sources of grouped module C_31C4_3986 and keep their original ids. */

#include "LAYOUT.H"
#include "R3E8.H"
#include "GB3AF.H"
#include "C470.H"

extern struct record3e8 g43b4[];

extern int g072e, g00bc;
extern char far *ui_gfx_blob, *rect_queue_write_ptr;
extern void f1ecd(void);
extern void timer_deadline_wait(void);
extern void board_redraw_view();
extern void timer_deadline_arm();
extern void sprite_draw_cursor(void);
extern void timer_wait_ticks();
extern int puzzle_run();
extern void f4eeb(int n);
extern void f03b4();
extern void f039f();
extern void hud_panel_open();
extern void sound_stop_reset(void);
extern void board_redraw_paint(void);
extern int board_record_index;
extern unsigned char far *board_records;
extern char far *src, far *dst;               /* DS:C5CA -> DS:40C4 */
/*@SYM _src=0xC5CA kind=g key=storage_objects/M_2C0FA.phys*/
/*@SYM _dst=0x40C4 kind=g key=storage_objects/M_23BF4.phys*/
extern int gbc, g40ce;
extern void wipe(int x, int y, int w, int h, int x2, int y2);   /* 03B4, same entry as f03b4 under this alias */
/*@SYM _wipe=0x03B4 kind=f key=functions/F_03B4.entry*/
extern unsigned char far *record_table_root;
extern int f250c(), f32fa(), f6181();
extern void f03cc();
extern void fcaf1();
extern int g96;
extern char s9a5c[], s99da[], s9b6e[], s9ae0[];
extern void f2986();
extern unsigned char far *g96ea;
extern int g73e, g736, g738;
extern void anim_step_loop(int x, int y, int w, int h, int x2, int y2);
extern void energy_set(int n);
extern int confirm_quit_dialog(void);
extern char g8bfe;
extern void longjmp(char far *s, int n);

/* ---- F_31C4 (original code at 0x31C4) ---- */
/* F_31C4 -- scroll the board into view, redraw the frame, scroll it out.
   si is the row cursor in both loops; ui_gfx_blob is copied into rect_queue_write_ptr as a far
   pointer (les bx / mov es / mov bx). */
void board_scroll_transition(void)
{
    register int y;

    if (g072e <= 8) {
        rect_queue_write_ptr = ui_gfx_blob;
        for (y = 0x10; y < 0x14; y++) {
            timer_deadline_arm(0x30);
            g072e = y;
            board_redraw_view();
            sprite_draw_cursor();
            f1ecd();
            timer_deadline_wait();
        }
        timer_wait_ticks(0x30);
    }
    g00bc = 0;
    sound_stop_reset();
    puzzle_run();
    board_redraw_paint();
    f03b4(8, 0xc8, 0x130, 0x90, 8, 0x10);
    f4eeb(0);
    sprite_draw_cursor();
    f039f(8, 0x10, 0x130, 0x90);
    hud_panel_open();
    g00bc = 1;
    rect_queue_write_ptr = ui_gfx_blob;
    if (g072e == 0x13) {
        for (y = 0x13; y >= 0x10; y--) {
            timer_deadline_arm(0x30);
            g072e = y;
            board_redraw_view();
            sprite_draw_cursor();
            f1ecd();
            timer_deadline_wait();
        }
        g072e = 0;
        timer_wait_ticks(0x30);
    }
}


/* ---- F_329F (original code at 0x329F) ---- */
void f329f(void)
{
    board_records = (char far *) &g43b4[board_record_index];
    board_redraw_paint();
    dst = src;
    wipe(8, 0xc8, 0x130, 0x90, 8, 0x10);
    g40ce = gbc = 0;
    f4eeb(0);
    gbc = 1;
}


/* ---- F_32FA (original code at 0x32FA) ---- */
f32fa(a) int a;{register int i;int j;int x,y,n;unsigned char *p;p=record_table_root+a*4+1;p[3]=(p[3]+4)&7;n=p[2]+2;x=p[0];y=p[1];j=(x+2)/4+((y+4)/8-2)*38-1;for(i=0;i<n;i++,j++)board_records[j]^=16;}


/* ---- F_338A (original code at 0x338A) ---- */
/* F_338A -- dispatch one scripted event stream and animate its unit. */
void board_run_unit_script(unsigned char far *s)
{
    int w, v, u, i, n, k;
    unsigned char c;
    unsigned char far *r;
    unsigned char far *q;
    register int x, y;

    n = *s;
    if (s[1] < 2) {
        x = s[2];
        x <<= 1;
        y = s[3];
        if (s[1] != 0) {
            x = s[2];
            x <<= 1;
            y = s[3];
            f03b4(x, y + 0x148, 0x18, 8, x, y + 0xb8);
            g96 = 0x167;
            if (s[4] ^= 1)
                f03cc(x, y + 0xb8, s9a5c, 0);
            else
                f03cc(x, y + 0xb8, s99da, 0);
            g96 = 0x9f;
            f03b4(x, y + 0xb8, 0x18, 8, x, y);
        } else {
            x = s[2];
            x <<= 1;
            y = s[3];
            f03b4(x, y + 0x148, 0x18, 9, x, y + 0xb8);
            g96 = 0x167;
            if (s[4] ^= 1)
                f03cc(x, y + 0xb8, s9b6e, 0);
            else
                f03cc(x, y + 0xb8, s9ae0, 0);
            g96 = 0x9f;
            f03b4(x, y + 0xb8, 0x18, 9, x, y);
        }
        fcaf1(8);
    } else if (s[1] == 2)
        fcaf1(0x16);
    s += i = 5;
    for (; i < n; i++, s++) {
        c = *s;
        if ((c & 0x80) == 0) {
            if (c & 0x10)
                f32fa(c & 0xf);
            else if (c & 0x40)
                f6181(c & 0xf);
            else
                f250c(c);
        } else if ((c & 0x30) == 0) {
            i++;
            s++;
            r = g43b4[*s].bytes + (c & 0x7f) * 3 + 0x2ac;
            c = *r;
            *r &= 0xf0;
            *r ^= 0x20;
            if (c & 0x80) {
                if (c & 0x20)
                    r[2] -= 0x30;
                else
                    r[2] += 0x30;
            } else {
                if (c & 0x20)
                    r[1] -= 0x18;
                else
                    r[1] += 0x18;
            }
        } else if (c & 0x10) {
            i++;
            s++;
            r = (q = g43b4[*s].bytes) + (c & 0xf) * 4 + 0x2cb;
            asm les bx,r
            asm mov al,es:[bx+3]
            asm neg ax
            asm les bx,r
            asm mov es:[bx+3],al
            c = r[2] + 2;
            w = r[0];
            v = r[1];
            u = (w + 2) / 4 + ((v + 4) / 8 - 2) * 0x26 - 1;
            for (k = 0; c > k; k++, u++)
                q[u] ^= 0x10;
        } else if (c & 0x20) {
            i++;
            s++;
            c = *s;
            ((unsigned char near *) gb3af)[c << 5] = 0;
        }
    }
}


/* ---- F_36F0 (original code at 0x36F0) ---- */
/* F_36F0 -- advance every queued 12-byte move record by one step: either
   finish the move (swap the from/to cells, repaint, re-mark the board) or
   redraw the unit at its current cell. */
void board_advance_unit_moves(int a)
{
    int n;
    int i;
    unsigned char far *p;
    unsigned char s;
    unsigned char cx;
    unsigned char cy;
    register int u, k;

    g96 = 0x190;
    fcaf1(0xa);
    a++;
    n = *g96ea;
    p = g96ea + 1;
    for (i = 0; i < n; i++, p += 0xc) {
        s = p[4];
        cx = p[0];
        cy = p[1];
        if (p[s + 5] != a) {
            if (s != 0) {
                p[4] = 0;
                f2986(p);
                f03b4(cx * 2, cy + 0xb8, 0x38, 0x10, cx * 2, cy);
            }
        } else {
            p[4] = ++s;
            if (s == 5 || p[s + 5] == 0) {
                p[4] = 0;
                u = p[0] / 4 + (p[1] / 8 - 1) * 0x26 - 1;
                for (k = 0; k < 6; k++, u++)
                    board_records[u] = board_records[u + 0x26] = 0;
                p[0] = p[2];
                p[1] = p[3];
                p[2] = cx;
                p[3] = cy;
                f03b4(cx * 2, cy + 0x148, 0x38, 0x10, cx * 2, cy);
                f03b4(cx * 2, cy + 0x148, 0x38, 0x10, cx * 2, cy + 0xb8);
                f2986(p);
                cx = p[0];
                cy = p[1];
                f03b4(cx * 2, cy + 0xb8, 0x38, 0x10, cx * 2, cy);
                u = cx / 4 + (cy / 8 - 1) * 0x26 - 1;
                for (k = 0; k < 6; k++, u++)
                    board_records[u] = board_records[u + 0x26] = 7;
            } else {
                f2986(p);
                f03b4(cx * 2, cy + 0xb8, 0x38, 0x10, cx * 2, cy);
            }
        }
    }
    g96 = 0x9f;
}


/* ---- F_3986 (original code at 0x3986) ---- */
void f3986(void)
{
    int si, di, w, h;

    g73e = board_record_index;
    g96 = 0x190;
    f4eeb(0xb8);
    g96 = 0x9f;

    if (g736 < 8) {
        si = 8;
        w = 0x28;
    } else if (g736 + 0x27 > 0x137) {
        si = g736;
        w = 0x138 - g736;
    } else {
        si = g736;
        w = 0x28;
    }

    if (g738 < 0x10) {
        di = 0x10;
        h = 0x28;
    } else if (g738 + 0x27 > 0x9f) {
        di = g738;
        h = 0xa0 - g738;
    } else {
        di = g738;
        h = 0x28;
    }

    anim_step_loop(si, di + 0xb8, w, h, si, di);

    slot_table[current_slot].value++;
    energy_set(slot_table[current_slot].state = 4);
    if (confirm_quit_dialog())
        longjmp((char far *) &g8bfe, 2);
}
