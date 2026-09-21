/* src/FONT.C: Font sheet selection and proportional text measurement/wrapping.
   One translation unit; the sections below were the separate member
   sources of grouped module C_6CA6_6D3C and keep their original ids. */

/* ---- F_6CA6 (original code at 0x6CA6) ---- */
/* F_6CA6 -- index the far resource table and publish three spans. */
extern unsigned char far *gc0d6[];
#include "LAYOUT.H"
extern unsigned gc0de,gc0e0,gc0e2,gc0e4,gc0e6,gc0ea;
void sprite_sheet_select(i)
int i;
{
    asm mov bx,[bp+4]
    asm mov [gc0ea],bx
    asm shl bx,1
    asm shl bx,1
    asm les si,[bx+gc0d6]
    asm mov ax,es
    asm mov [gc0e0],ax
    asm xor ax,ax
    asm mov cx,ax
    asm mov cl,es:[si+1]
    asm inc cx
    asm mov al,es:[si+2]
    asm mov [dialog_line_height],ax
    asm add si,3
    asm mov [gc0e4],si
    asm add si,cx
    asm mov [gc0e6],si
    asm add si,cx
    asm mov [gc0e2],si
    asm add si,cx
    asm mov [gc0de],si
}


/* ---- F_6CEA (original code at 0x6CEA) ---- */
/* F_6CEA -- read back the word at DS:C0EA (the index F_6CA6 latched there).
   The trailing EB00 is the `return`'s jump to the epilogue at zero
   displacement (tc20-codegen rule 7's shape, here for a return). */
extern unsigned gc0ea;                  /* DS:C0EA */

int sprite_sheet_index_get()
{
    return (gc0ea);
}


/* ---- F_6CF0 (original code at 0x6CF0) ---- */
#include "LAYOUT.H"
int dialog_line_height_get(void)
{
    return dialog_line_height;
}


/* ---- F_6CF6 (original code at 0x6CF6) ---- */
/* F_6CF6 -- the width in pixels of the widest line of a string, in the
   proportional font whose width table is at ES:DI (segment DS:C0E0, offset
   DS:C0E4).  A TC frame around an `asm` body, the F_1F17 shape: TC 2.0
   emits the SI and DI saves itself because the asm names them (rule 14),
   and only the DS save is written here.  Every forward jump is spelled
   `short` so TASM's two-pass forward-reference padding cannot widen it. */
extern unsigned gc0e0;                  /* DS:C0E0, the font segment */
extern unsigned gc0e4;                  /* DS:C0E4, the width table offset */

int text_line_width(s)
char far *s;
{
    asm push ds
    asm cld
    asm xor dx,dx
    asm mov cx,dx
    asm mov bx,dx
    asm mov ax,gc0e0
    asm mov es,ax
    asm mov di,gc0e4
    asm lds si,[bp+4]
L0: asm lodsb
    asm or al,al
    asm jz short L4
    asm cmp al,0Ah
    asm jz short L2
    asm cmp al,0Dh
    asm jz short L2
    asm mov bl,al
    asm mov bl,es:[bx+di]
    asm add dx,bx
    asm jmp short L0
L2: asm cmp dx,cx
    asm jna short L3
    asm mov cx,dx
L3: asm xor dx,dx
    asm jmp short L0
L4: asm cmp dx,cx
    asm jna short L5
    asm mov cx,dx
L5: asm mov ax,cx
    asm pop ds
}


/* ---- F_6D3C (original code at 0x6D3C) ---- */
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
