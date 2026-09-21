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
