/* F_CB5C -- mechanically split complete routine. */
void options_toggle_option()
{
    asm db 056h,0a1h,0edh,013h,0bah,01bh,000h,0f7h,0e2h,08bh,0d8h,081h,0c3h,070h,0c4h,01eh,007h,026h,08bh,047h,011h,0f7h,0d8h,01bh
    asm db 0c0h,040h,0a2h,093h,021h,01eh,0b8h,088h,021h,050h,0e8h,048h,0bbh,059h,059h,08bh,0f0h,00bh,0f6h,07ch,02eh,0a1h,0edh,013h
    asm db 0bah,01bh,000h,0f7h,0e2h,08bh,0d8h,081h,0c3h,070h,0c4h,01eh,007h,026h,089h,077h,011h,00bh,0f6h,074h,016h,0a1h,0edh,013h
    asm db 0bah,01bh,000h,0f7h,0e2h,08bh,0d8h,081h,0c3h,070h,0c4h,01eh,007h,026h,0c7h,047h,013h,0ffh,0ffh,033h,0c0h,0ebh,000h,05eh
    asm db 0c3h
    asm _f_cb5c_end label byte
    asm public _f_cb5c_end
}
