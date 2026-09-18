/* F_6D3C -- draw a NUL-terminated far string, advancing x by whatever f03c6
   returns per glyph and wrapping to the left margin on CR or LF. */
extern int f03c6();
extern unsigned gc0e8;                  /* DS:C0E8, the line height */

void f6d3c(x, y, s)
int x;
int y;
char far *s;
{
    char c;
    register int px;

    px = x;
    while (c = *s++) {
        if (c == 0xa || c == 0xd) {
            y += gc0e8;
            px = x;
        } else {
            px += f03c6(px, y, c);
        }
    }
}
