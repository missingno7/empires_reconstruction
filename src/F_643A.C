/* F_643A -- update and rewrite one indexed data-file record. */
extern unsigned gb40, gb3e, gc0c9;
extern unsigned char gc0c8;
extern unsigned gb33, gb31, gb2a;
extern void f6266(), f652a(), f86c9();
extern int lseek(), read(), write(), close();

void f643a()
{
    asm db 055h,08bh,0ech,083h,0ech,0ch,056h,057h
    asm db 08bh,076h,4,08bh,0feh,0b1h,0ch,0d3h,0efh
    asm db 0a1h
    asm dw gb40
    asm db 089h,046h,0feh,081h,0e6h,0ffh,0fh
    asm db 0c7h,06h
    asm dw gb40
    asm dw 1
    asm db 0c7h,06h
    asm dw gb3e
    asm dw 0
    asm db 057h
    asm call near ptr f6266
    asm db 059h,033h,0c0h,050h,08bh,0c6h,0d1h,0e0h,0d1h,0e0h
    asm db 033h,0d2h,052h,050h,0ffh,036h
    asm dw gc0c9
    asm call near ptr lseek
    asm db 083h,0c4h,8,0b8h,4,0,050h,016h,08dh,046h,0f6h,050h
    asm db 0ffh,036h
    asm dw gc0c9
    asm call near ptr read
    asm db 083h,0c4h,8,0b8h,4,0,050h,016h,08dh,046h,0fah,050h
    asm db 0ffh,036h
    asm dw gc0c9
    asm call near ptr read
    asm db 083h,0c4h,8,08bh,046h,0fah,02bh,046h,0f6h,089h,046h,0f4h
    asm db 033h,0c0h,050h,08bh,056h,0f8h,08bh,046h,0f6h,05h,2,0,083h,0d2h,0
    asm db 052h,050h,0ffh,036h
    asm dw gc0c9
    asm call near ptr lseek
    asm db 083h,0c4h,8,08bh,046h,0f4h,05h,0feh,0ffh,050h
    asm db 0ffh,076h,8,0ffh,076h,6,0ffh,036h
    asm dw gc0c9
    asm call near ptr write
    asm db 083h,0c4h,8,0ffh,036h
    asm dw gc0c9
    asm call near ptr close
    asm db 059h,083h,03eh
    asm dw gb3e
    asm db 0,074h,026h,0a0h
    asm dw gc0c8
    asm db 098h,050h
    asm call near ptr f652a
    asm db 059h,08ch,01eh
    asm dw gb33
    asm db 0c7h,06h
    asm dw gb31
    asm db 0c7h,0ah,01eh,0b8h,02ah,0bh,050h
    asm call near ptr f86c9
    asm db 059h,059h,0a0h
    asm dw gc0c8
    asm db 098h,050h
    asm call near ptr f652a
    asm db 059h,083h,03eh
    asm dw gb3e
    asm db 0,074h,3,0e9h,03eh,0ffh,08bh,046h,0feh,0a3h
    asm dw gb40
    asm db 0fbh,05fh,05eh,08bh,0e5h,05dh
}
