/* F_6B4A -- non-blocking INT 16h keyboard poll. */
int f6b4a()
{
    asm mov ah,1
    asm int 16h
    asm jz  L_none
    asm or  al,al
    asm jnz L_ascii
    asm mov al,ah
    asm mov ah,1
    asm jmp short L_out
L_ext: asm mov ax,100h
    asm jmp short L_ascii
L_none: asm xor ax,ax
    asm jmp short L_out
L_ascii: asm xor ah,ah
L_out: ;
}
