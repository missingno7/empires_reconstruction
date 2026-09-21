/* src/KEYBIOS.C: BIOS keyboard helpers (INT 16h): blocking read with the F1..F10
   hot-key dispatch and the non-consuming poll. One translation unit; the two
   sections were the separate exact C reconstructions F_6B1A.C and F_6B4A.C. */

/* ---- F_6B1A ---- */
/* F_6B1A -- blocking INT 16h read with the F1..F10 hot-key check. */
extern int menu_list_active();
extern void menu_loop_run();

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
    asm call near ptr menu_list_active
    asm or  ax,ax
    asm mov ax,si
    asm jz  L_out
    asm sub ax,013bh
    asm push ax
    asm call near ptr menu_loop_run
    asm pop ax
    asm mov ax,si
    asm jmp short L_out
L_ascii: asm xor ah,ah
L_out: ;
}

/* ---- F_6B4A ---- */
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
