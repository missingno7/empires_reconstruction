/* F_53BF -- select the display mode, including the VGA probe path. */
extern unsigned g1778;
extern unsigned gbfcd;
extern void fe54d();
void f53bf()
{
    asm mov g1778,0
    asm cmp word ptr gbfcd,3
    asm db 075h,08h
    asm mov g1778,1
    asm db 0ebh,35h
    asm call near ptr fe54d
    asm or ax,ax
    asm db 074h,08h
    asm mov g1778,2
    asm db 0ebh,26h
    asm mov ah,0c0h
    asm int 15h
    asm or ah,ah
    asm db 075h,01eh
    asm db 026h,081h,07fh,2,0fch,0bh
    asm db 075h,016h
    asm mov cx,0ah
    asm mov dx,203h
    asm mov al,0a5h
    asm out dx,al
    asm in al,dx
    asm cmp al,0a5h
    asm db 075h,08h
    asm db 0e2h,0f6h
    asm mov g1778,3
}
