/* F_98CB -- mechanically split complete routine. */
void f_98cb()
{
    asm db 0b8h,068h,001h,050h,0b8h,050h,000h,050h,0b8h,01dh,000h,050h,0b8h,028h,000h,050h,0b8h,068h,001h,050h,0b8h,001h,000h,050h
    asm db 0e8h,0d1h,06ah,083h,0c4h,00ch,0ffh,036h,050h,0c3h,0ffh,036h,04eh,0c3h,0b8h,01dh,000h,050h,0b8h,028h,000h,050h,0b8h,068h
    asm db 001h,050h,0b8h,050h,000h,050h,0e8h,0b0h,06ah,083h,0c4h,00ch,0c3h
    asm _f_98cb_end label byte
    asm public _f_98cb_end
}
