/* src/FONT.C: Font sheet selection and proportional text measurement/wrapping. */
#include "game.h"

/* ---- F_6CA6 (original code at 0x6CA6) ---- */
/* F_6CA6 -- index the far resource table and publish three spans.  Historical
   body was hand-written asm (comment at src/FONT.C:9-16): load the i-th far
   pointer out of gc0d6, latch it as the new font base, and publish four
   offsets relative to that base -- 3, 3+cx, 3+2*cx, 3+3*cx, where cx is the
   glyph-width-table stride (p[1]+1) and dialog_line_height is p[2].

   PORT: transcribed straight from the asm per tu-porting-rules.md sec 5
   ("sprite_sheet_select ... C reimplementation of the documented walk").
   The offsets are relative to gc0e0 itself (not to the far pointer's raw
   16-bit offset field) because gc0e0 is now the REAL base pointer
   (portable/include/gfx.h: "font resource base (was a segment)") and every
   reader indexes through it as gc0e0[gc0e4 + glyph] etc. (gfx.h, gfx_draw_char);
   folding the historical near-offset into gc0e0 itself would double count it. */
void sprite_sheet_select(dos_int i)
{
    const uint8_t *p;
    dos_uint cx;

    gc0ea = (dos_uint)i;
    p = gc0d6[i];
    cx = p[1] + 1;
    dialog_line_height = p[2];
    gc0e0 = p;
    gc0e4 = 3;
    gc0e6 = 3 + cx;
    gc0e2 = 3 + 2 * cx;
    gc0de = 3 + 3 * cx;
}


/* ---- F_6CEA (original code at 0x6CEA) ---- */
/* F_6CEA -- read back the word at DS:C0EA (the index F_6CA6 latched there). */
dos_int sprite_sheet_index_get()
{
    return (dos_int)gc0ea;
}


/* ---- F_6CF0 (original code at 0x6CF0) ---- */
dos_int dialog_line_height_get(void)
{
    return dialog_line_height;
}


/* ---- F_6CF6 (original code at 0x6CF6) ---- */
/* F_6CF6 -- the width in pixels of the widest line of a string, in the
   proportional font whose width table is at gc0e0[gc0e4 + c].  PORT:
   transcribed from the documented asm walk (src/FONT.C:70-101): dx (running
   width) resets to 0 on CR/LF, cx tracks the widest line seen so far via
   "if (dx > cx) cx = dx" at every line break and again at the terminating
   NUL, and each character's width is read unsigned (`mov bl,al` / `mov
   bl,es:[bx+di]` -- bh stays 0 from the initial `xor bx,bx`, so the byte
   value indexes the table zero-extended, matching (dos_uchar)c below). */
dos_int text_line_width(dos_char *s)
{
    dos_char c;
    dos_uint width;     /* dx */
    dos_uint max_width; /* cx */

    width = 0;
    max_width = 0;
    while ((c = *s++) != 0) {
        if (c == 0x0a || c == 0x0d) {
            if (width > max_width) max_width = width;
            width = 0;
            continue;
        }
        width += gc0e0[gc0e4 + (dos_uchar)c];
    }
    if (width > max_width) max_width = width;
    return (dos_int)max_width;
}


/* ---- F_6D3C (original code at 0x6D3C) ---- */
/* F_6D3C -- draw a NUL-terminated far string, advancing x by whatever gfx_draw_char
   returns per glyph and wrapping to the left margin on CR or LF. */
void text_draw_wrapped(dos_int x, dos_int y, dos_char *s)
{
    dos_char c;
    dos_int px;

    px = x;
    while ((c = *s++) != 0) {
        if (c == 0xa || c == 0xd) {
            y += dialog_line_height;
            px = x;
        } else {
            px += gfx_draw_char(px, y, c);
        }
    }
}
