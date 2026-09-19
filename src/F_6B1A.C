/* F_6B1A -- blocking INT 16h read with the F1..F10 hot-key check. */
extern void f792c();
extern void f7964();

int f6b1a()
{
    asm xor ah,ah
    asm int 16h
    asm or  al,al
    asm jnz L_ascii
    asm mov al,ah
    asm mov ah,1
    asm cmp al,3bh
    asm jb  L_out
    asm cmp al,44h
    asm jg  L_out
    asm mov si,ax
    asm call near ptr f792c
    asm or  ax,ax
    asm mov ax,si
    asm jz  L_out
    asm sub ax,013bh
    asm push ax
    asm call near ptr f7964
    asm pop ax
    asm mov ax,si
    asm jmp short L_out
L_ascii: asm xor ah,ah
L_out: ;
}
