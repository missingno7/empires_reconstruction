/* F_4EEB -- walk the record table and copy matching sprites.  DI-addressing
   and the historical register saves are byte-coded to prevent Turbo C from
   adding a different save pair around the inline-assembly body. */
extern void f01ce();
extern void f03a5();
extern void f03cc();
extern unsigned char gb3ae[];
extern unsigned int gbfba;
extern unsigned long gb07c[];

void f4eeb(y0)
int y0;
{
    asm mov ax,0fh
    asm push ax
    asm call near ptr f01ce
    asm pop ax
    asm db 056h
    asm db 057h
    asm db 0BFh
    asm dw offset DGROUP:gb3ae
    asm xor cx,cx
    asm db 08Ah,00Dh
    asm or cx,cx
    asm je L_end
    asm db 047h
L_loop: asm xor ax,ax
    asm db 08Ah,045h,001h
    asm cmp ax,gbfba
    asm jne L_skip
    asm db 080h,07Dh,008h,001h
    asm je L_skip
    asm push cx
    asm xor bx,bx
    asm db 08Ah,05Dh,007h
    asm push bx
    asm db 08Ah,05Dh,006h
    asm shl bx,1
    asm shl bx,1
    asm les ax,dword ptr gb07c[bx]
    asm push es
    asm push ax
    asm mov ax,[bp+4]
    asm db 003h,045h,004h
    asm push ax
    asm db 0FFh,075h,002h
    asm call near ptr f03cc
    asm add sp,0ah
    asm db 080h,07Dh,01Ah,000h
    asm je L_skip2
    asm db 08Bh,045h,002h
    asm add ax,10h
    asm db 08Bh,05Dh,004h
    asm db 08Ah,055h,01Ah
    asm xor dh,dh
    asm sub bx,dx
    asm inc bx
    asm push bx
    asm push dx
    asm push ax
    asm call near ptr f03a5
    asm add sp,6
L_skip2: asm pop cx
L_skip: asm db 083h,0C7h,020h
    asm loop L_loop
L_end: asm db 05Fh
    asm db 05Eh
}
