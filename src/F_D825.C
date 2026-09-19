/* F_D825 -- append one compact record to the DS:2F30 table. */
void fd825()
{
    asm db 055h,08bh,0ech,057h,08ch,0d8h,08eh,0c0h
    asm db 0bfh,030h,02fh,08ah,01dh,0feh,05h,047h
    asm db 08ah,0c3h,0d0h,0e0h,0d0h,0e0h,02h,0c3h
    asm db 02ah,0e4h,03h,0f8h,08ah,046h,04h,0aah
    asm db 08bh,05eh,08h,08bh,046h,06h,08ah,0e3h,0abh
    asm db 08bh,05eh,0ch,08bh,046h,0ah,08ah,0e3h,0abh
    asm db 08bh,0c7h,02dh,04h,0,05fh,05dh
}
