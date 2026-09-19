/* F_4E9F -- record-table walk and call to the rectangle clipper.
   The DI-addressing instructions are emitted as bytes so Turbo C does not
   add its own SI/DI save pair around the inline-assembly body. */
extern void f4aa8();
extern unsigned char gb3ae[];
extern unsigned int gbfba;
extern unsigned char gbf66[];
extern unsigned char g9bfc[];

void f4e9f()
{
    asm db 55h
    asm db 8Bh,0ECh
    asm db 56h
    asm db 57h
    asm db 0BFh
    asm dw offset DGROUP:gb3ae
    asm xor cx,cx
    asm db 08Ah,00Dh
    asm or cx,cx
    asm je L_out
    asm db 047h
L_top: asm xor ax,ax
    asm db 08Ah,045h,001h
    asm cmp ax,gbfba
    asm jne L_next
    asm db 080h,07Dh,008h,001h
    asm je L_next
    asm push cx
    asm xor ax,ax
    asm db 08Ah,045h,006h
    asm mov bx,ax
    asm mov al,[bx+gbf66]
    asm push ax
    asm mov al,[bx+g9bfc]
    asm push ax
    asm db 08Bh,045h,004h
    asm push ax
    asm db 08Bh,045h,002h
    asm push ax
    asm call near ptr f4aa8
    asm add sp,8
    asm pop cx
L_next: asm db 083h,0C7h,020h
    asm loop L_top
L_out: asm db 5Fh
    asm db 5Eh
    asm db 5Dh
}
