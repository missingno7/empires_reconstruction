/* F_6D3C -- draw a NUL-terminated far string, advancing x by whatever gfx_draw_char
   returns per glyph and wrapping to the left margin on CR or LF. */
#include "LAYOUT.H"
#include "VIDEO.H"

void text_draw_wrapped(x, y, s)
int x;
int y;
char far *s;
{
    char c;
    register int px;

    px = x;
    while (c = *s++) {
        if (c == 0xa || c == 0xd) {
            y += dialog_line_height;
            px = x;
        } else {
            px += gfx_draw_char(px, y, c);
        }
    }
}
