/* F_4AA8 -- clip a rectangle and hand it to the wipe routine. */
extern void gfx_wipe_rect();

void play_window_wipe_clipped(x1, y1, w, h)
int x1, y1, w, h;
{
    asm mov ax,[bp+4]
    asm cmp ax,137h
    asm jg  L_out
    asm mov bx,[bp+6]
    asm cmp bx,9Fh
    asm jg  L_out
    asm mov cx,[bp+8]
    asm add cx,ax
    asm dec cx
    asm cmp cx,8
    asm jl  L_out
    asm mov dx,[bp+0Ah]
    asm add dx,bx
    asm dec dx
    asm cmp dx,10h
    asm jl  L_out
    asm cmp ax,8
    asm jge L_x1ok
    asm mov ax,8
L_x1ok: asm cmp cx,137h
    asm jle L_x2ok
    asm mov cx,137h
L_x2ok: asm cmp bx,10h
    asm jge L_y1ok
    asm mov bx,10h
L_y1ok: asm cmp dx,9Fh
    asm jle L_y2ok
    asm mov dx,9Fh
L_y2ok: asm sub cx,ax
    asm inc cx
    asm sub dx,bx
    asm inc dx
    asm push bx
    asm push ax
    asm push dx
    asm push cx
    asm add bx,0B8h
    asm push bx
    asm push ax
    asm call near ptr gfx_wipe_rect
    asm add sp,0Ch
L_out: ;
}
