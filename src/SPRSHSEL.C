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

