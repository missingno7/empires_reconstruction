/* F_CC17 -- mechanically split complete routine. */
void options_toggle_sound()
{
    asm db 056h,0a1h,0edh,013h,0bah,01bh,000h,0f7h,0e2h,08bh,0d8h,081h,0c3h,070h,0c4h,01eh,007h,026h,08bh,047h,00dh,0f7h,0d8h,01bh
    asm db 0c0h,040h,0a2h,07fh,021h,01eh,0b8h,074h,021h,050h,0e8h,08dh,0bah,059h,059h,08bh,0f0h,00bh,0f6h,07ch,022h,08bh,0c6h,050h
    asm db 0a1h,0edh,013h,0bah,01bh,000h,0f7h,0e2h,08bh,0d8h,081h,0c3h,070h,0c4h,01eh,007h,058h,026h,089h,047h,00dh,0a3h,06eh,017h
    asm db 00bh,0f6h,075h,003h,0e8h,0e2h,0feh,033h,0c0h,0ebh,000h,05eh,0c3h
    asm _f_cc17_end label byte
    asm public _f_cc17_end
}
