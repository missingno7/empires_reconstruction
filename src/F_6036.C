/* F_6036 -- traverse sprite records and append their compact draw records. */
extern unsigned char far *sprbase;
extern unsigned char far *objtab;
extern void fd825();
extern void f03cc();
extern unsigned g0a20;

void f6036()
{
    asm db 055h,08bh,0ech,056h,057h,01eh,0fch,0bdh,02fh,0
    asm db 0c4h,03eh
    asm dw sprbase
    asm db 0c5h,036h
    asm dw objtab
    asm db 0ach,032h,0edh,08ah,0c8h,057h,045h,0adh,08bh,0d8h
    asm db 0ach,032h,0e4h,024h,01fh,0bah,0e6h,01h,0f7h,0e2h
    asm db 05h,2,0,03h,0f8h,033h,0d2h,08ah,0d7h,08ah,0feh
    asm db 01eh,051h,006h
    asm mov ax,DGROUP
    asm db 08eh,0d8h,0b8h,01eh,0
    asm db 050h,0b8h,0fh,0,050h,052h,053h,055h
    asm call near ptr fd825
    asm db 058h,05bh,05ah,083h,0c4h,4,07h,06h,033h,0c0h,050h
    asm db 06h,057h,081h,0c2h,0b8h,0,052h,0d1h,0e3h,053h
    asm call near ptr f03cc
    asm db 083h,0c4h,0ah,07h,059h,01fh,05fh,0e2h,0afh
    asm db 0c7h,06h
    asm dw g0a20
    asm dw 0ah
    asm db 01fh,05fh,05eh,05dh
}
