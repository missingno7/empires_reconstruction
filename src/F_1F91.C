/* F_1F91 -- clamp a map span, then OR the bytes in its 26h-stride rows. */
extern unsigned long gbfbc;

void f1f91(x,y,n)
int x,y,n;
{
    asm push ds
    asm lds si,dword ptr gbfbc
    asm mov ax,[bp+4]
    asm cmp ax,137h
    asm jg L_xhi
    asm cmp ax,8
    asm jge L_xok
    asm mov ax,8
    asm jmp short L_xok
L_xhi: asm mov ax,137h
L_xok: asm mov bx,[bp+6]
    asm cmp bx,9Fh
    asm jg L_yhi
    asm cmp bx,10h
    asm jge L_yok
    asm mov bx,10h
    asm jmp short L_yok
L_yhi: asm mov bx,9Fh
L_yok: asm shr ax,1
    asm shr ax,1
    asm shr ax,1
    asm mov cx,[bp+8]
    asm add cx,bx
    asm dec cx
    asm shr bx,1
    asm shr bx,1
    asm shr bx,1
    asm shr cx,1
    asm shr cx,1
    asm shr cx,1
    asm cmp cx,13h
    asm jl L_nlo
    asm mov cx,13h
L_nlo: asm sub cx,bx
    asm inc cx
    asm sub bx,2
    asm dec ax
    asm shl bx,1
    asm mov di,bx
    asm shl bx,1
    asm add di,bx
    asm shl bx,1
    asm shl bx,1
    asm shl bx,1
    asm add bx,di
    asm add bx,ax
    asm add si,bx
    asm xor ax,ax
L_row: asm or al,[si]
    asm add si,26h
    asm loop L_row
    asm pop ds
}
