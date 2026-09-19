/* F_652A -- read three sectors through the BIOS disk service. */
extern unsigned char gc0c8;

void f652a()
{
    asm db 055h,08bh,0ech,081h,0ech,0,2,056h,033h,0f6h
    asm db 0ebh,028h,0b4h,0
    asm db 08ah,016h
    asm dw gc0c8
    asm db 0cdh,013h,08ch,0d2h,08dh,086h,0,0feh,08eh,0c2h
    asm db 08ch,0d2h,08dh,086h,0,0feh,08bh,0d8h,0b4h,2
    asm db 0b0h,1,0b5h,1,0b1h,1,0b6h,0,08ah,056h,4,0cdh,013h
    asm db 46h,083h,0feh,3,07ch,0d3h,0b4h,0dh,0cdh,021h
    asm db 05eh,08bh,0e5h,05dh
}
