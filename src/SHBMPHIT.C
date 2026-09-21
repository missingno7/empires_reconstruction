/* F_1F17 -- collision probe against the shadow bitmap.  TC frame + asm body:
   TC 2.01 emits the si/di saves ITSELF for a function whose asm mentions
   si/di, so the body must NOT repeat them; only push ds/pop ds are written. */
extern char far *vram;                  /* DS:BFBC, far pointer */
/*@SYM _vram=0xBFBC kind=g key=storage_objects/M_2BAEC.phys*/

int shadow_bitmap_hit_test(x, y, w)
int x, y, w;
{
    asm push ds
    asm lds si,dword ptr vram
    asm mov ax,[bp+4]
    asm cmp ax,137h
    asm jg  L1
    asm cmp ax,8
    asm jge L2
    asm mov ax,8
    asm jmp short L2
L1: asm mov ax,137h
L2: asm mov bx,[bp+6]
    asm cmp bx,9fh
    asm jg  L3
    asm cmp bx,10h
    asm jge L4
    asm mov bx,10h
    asm jmp short L4
L3: asm mov bx,9fh
L4: asm shr ax,1
    asm mov cx,[bp+8]
    asm add cx,ax
    asm dec cx
    asm shr ax,1
    asm shr ax,1
    asm shr cx,1
    asm shr cx,1
    asm cmp cx,26h
    asm jl  L5
    asm mov cx,26h
L5: asm sub cx,ax
    asm inc cx
    asm shr bx,1
    asm shr bx,1
    asm shr bx,1
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
L6: asm or  al,[si]
    asm inc si
    asm loop L6
    asm pop ds
}
