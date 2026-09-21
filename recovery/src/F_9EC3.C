/* F_9EC3 -- copy a clipped 16-pixel row through two packed lookup tables. */
extern unsigned char far *g3924;

void anim_step_row_copy()
{
    asm db 055h,08bh,0ech,083h,0ech,2,056h,057h,01eh,0fch
    asm db 08bh,05eh,0eh,0d1h,0e3h,0d1h,0e3h,0c4h,0bfh
    asm dw g3924
    asm db 03h,07eh,0ch,08bh,05eh,6,0d1h,0e3h,0d1h,0e3h
    asm db 0c5h,0b7h
    asm dw g3924
    asm db 03h,076h,4,08bh,056h,8,08bh,04eh,0ah,08bh,046h,012h
    asm db 02bh,0c2h,089h,046h,0feh,08bh,046h,010h,01eh
    asm mov bx,DGROUP
    asm db 08eh,0dbh,08dh,01eh,0b0h,012h,0d7h,01fh,01eh
    asm mov bx,DGROUP
    asm db 08eh,0dbh,08dh,01eh,0c0h,012h,0d7h,01fh,03bh,0d0h
    asm db 07eh,014h,03h,0f0h,03h,0f8h,0a4h,083h,0c6h,0fh
    asm db 02bh,0f0h,083h,0c7h,0fh,02bh,0f8h,083h,0eah,010h,07fh,0e8h
    asm db 03h,0f2h,03h,0fah,08bh,056h,8,03h,076h,0feh,03h,07eh,0feh
    asm db 0e2h,0cdh,01fh,05fh,05eh,08bh,0e5h,05dh
}
