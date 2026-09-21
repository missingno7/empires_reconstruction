/* F_6181 -- adjust a sprite record and emit its two draw segments. */
extern unsigned char far *sprbase;
extern unsigned char far *objtab;
extern void gfx_wipe_rect();
extern void gfx_copy_rect();

void f6181()
{
    asm db 055h,08bh,0ech,056h,057h,01eh,0fch,0c4h,03eh
    asm dw sprbase
    asm db 0c5h,036h
    asm dw objtab
    asm db 46h,08bh,046h,4,08bh,0d8h,0d1h,0e0h,03h,0f0h,03h,0f3h
    asm db 0adh,033h,0d2h,08ah,0d4h,08ah,0feh,08ah,0d8h,0d1h,0e3h,08bh,0eah
    asm db 08ah,04h,032h,0e4h,0a8h,040h,074h,0ah,024h,01fh,0feh,0c8h,07dh,02h
    asm db 0b0h,017h,0ebh,0ah,024h,01fh,0feh,0c0h,03ch,017h,07eh,02h,0b0h,0
    asm db 080h,024h,0e0h,08h,04h,0bah,0e6h,01h,0f7h,0e2h,05h,2,0
    asm db 03h,0f8h,08bh,0d5h,006h,052h
    asm mov ax,DGROUP
    asm db 08eh,0d8h,052h,08bh,0ech,053h,0b8h,01eh,0,050h,0b8h,01eh,0,050h
    asm db 08bh,0c2h,05h,048h,01h,050h,053h
    asm call near ptr gfx_wipe_rect
    asm db 081h,046h,0,0b8h,0
    asm call near ptr gfx_wipe_rect
    asm db 05bh,083h,0c4h,0ah,05ah,007h,033h,0c0h,050h,006h,057h,052h,053h
    asm call near ptr gfx_copy_rect
    asm db 05bh,05ah,083h,0c4h,6,08bh,0c2h,05h,0b8h,0,050h,053h
    asm db 0b8h,01eh,0,050h,0b8h,01eh,0,050h,052h,053h
    asm call near ptr gfx_wipe_rect
    asm db 083h,0c4h,0ch,01fh,05fh,05eh,05dh
}
