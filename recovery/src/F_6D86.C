/* F_6D86 -- PackBits-style far RLE stage. */
void rle_packbits_decode(src,dst,srclen)
int src,dst,srclen;
{
    asm push es
    asm push ds
    asm cld
    asm lds si,[bp+4]
    asm les di,[bp+8]
    asm mov bx,si
    asm mov dx,di
    asm mov ax,[bp+0Ch]
    asm add bx,ax
L_item: asm lodsb
    asm cbw
    asm cmp ax,0
    asm jg L_lit
    asm neg ax
    asm inc ax
    asm mov cx,ax
    asm lodsb
    asm rep stosb
    asm cmp si,bx
    asm jge L_done
    asm jmp short L_item
L_lit: asm mov cx,ax
    asm rep movsb
    asm cmp si,bx
    asm jl L_item
L_done: asm mov ax,di
    asm sub ax,dx
    asm clc
    asm pop ds
    asm pop es
}
