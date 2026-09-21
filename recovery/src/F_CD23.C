/* F_CD23 -- mechanically split complete routine. */
void slot_list_show()
{
    asm db 055h,08bh,0ech,083h,0ech,002h,056h,057h,0b8h,001h,000h,050h,01eh,0b8h,0d8h,021h,050h,0e8h,048h,0b7h,083h,0c4h,006h,033h
    asm db 0c0h,050h,0e8h,08dh,034h,059h,0b8h,027h,000h,050h,0b8h,032h,000h,050h,0b8h,02bh,000h,050h,0e8h,051h,036h,083h,0c4h,006h
    asm db 0b8h,026h,000h,050h,0b8h,032h,000h,050h,0b8h,099h,000h,050h,0e8h,03fh,036h,083h,0c4h,006h,0b8h,037h,000h,050h,0b8h,032h
    asm db 000h,050h,0b8h,0dch,000h,050h,0e8h,02dh,036h,083h,0c4h,006h,0e8h,075h,09fh,089h,046h,0feh,033h,0f6h,033h,0ffh,0ebh,023h
    asm db 0b8h,001h,000h,050h,08bh,0c7h,005h,034h,000h,050h,01eh,08bh,0c6h,0bah,01bh,000h,0f7h,0e2h,005h,070h,0c4h,050h,0e8h,0f0h
    asm db 0d4h,083h,0c4h,008h,046h,08bh,046h,0feh,040h,003h,0f8h,083h,0feh,00ah,07ch,0d8h,0b8h,0c8h,000h,050h,0b8h,030h,001h,050h
    asm db 033h,0c0h,050h,0b8h,008h,000h,050h,0e8h,0e1h,035h,083h,0c4h,008h,0e8h,056h,09dh,08bh,0f0h,083h,0feh,01bh,074h,005h,083h
    asm db 0feh,00dh,075h,0f1h,0e8h,080h,0b6h,033h,0c0h,0ebh,000h,05fh,05eh,08bh,0e5h,05dh,0c3h
    asm _f_cd23_end label byte
    asm public _f_cd23_end
}
