/* F_D89A -- return the first rectangle-table id overlapping the query. */
extern unsigned char g2f30[];
void fd89a(x,y,w,h)
int x,y,w,h;
{
    asm cld
    asm mov ax,[bp+4]
    asm mov bl,al
    asm mov ax,[bp+6]
    asm mov bh,al
    asm mov ax,[bp+8]
    asm mov dl,al
    asm mov ax,[bp+0Ah]
    asm mov dh,al
    asm add dx,bx
    asm dec dl
    asm dec dh
    asm mov si,offset DGROUP:g2f30
    asm lodsb
    asm sub ch,ch
    asm mov cl,al
    asm or cx,cx
    asm je L_zero
L_top: asm sub ah,ah
    asm lodsb
    asm mov di,ax
    asm lodsw
    asm cmp dl,al
    asm jb L_skip
    asm cmp dh,ah
    asm jb L_skip
    asm mov bp,ax
    asm lodsw
    asm add ax,bp
    asm cmp al,bl
    asm jbe L_next
    asm cmp ah,bh
    asm jbe L_next
    asm mov ax,di
    asm jmp short L_done
L_skip: asm add si,2
L_next: asm loop L_top
L_zero: asm sub ax,ax
L_done: ;
}
