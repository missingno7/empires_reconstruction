/* F_6EFF -- mask packed nibbles and expand the following byte stream. */
void f6eff()
{
    asm db 055h,08bh,0ech,056h,057h,01eh,0fch,0c4h,07eh,04h
    asm db 08ch,0c0h,08eh,0d8h,08bh,0dfh,0b9h,8,0
    asm db 026h,08bh,05h,025h,0fh,0fh,0abh,0e2h,0f7h
    asm db 083h,0c7h,010h,08bh,0f7h,0adh,0f6h,0e4h,08bh,0c8h
    asm db 08bh,0feh,0ach,08ah,0e0h,025h,0fh,0f0h,0d7h
    asm db 086h,0e0h,0d0h,0e8h,0d0h,0e8h,0d0h,0e8h,0d0h,0e8h
    asm db 0d7h,0d0h,0e0h,0d0h,0e0h,0d0h,0e0h,0d0h,0e0h
    asm db 0ah,0c4h,0aah,0e2h,0e1h,01fh,05fh,05eh,05dh
}
