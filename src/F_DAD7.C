/* F_DAD7 -- mechanically split complete routine. */
void f_dad7()
{
    asm db 055h,08bh,0ech,056h,083h,07eh,006h,000h,074h,005h,0b8h,07fh,000h,0ebh,002h,033h,0c0h,089h,046h,006h,08bh,05eh,004h,0d1h
    asm db 0e3h,081h,0c3h,0d2h,02fh,01eh,007h,026h,08ah,047h,001h,098h,08bh,0f0h,083h,07eh,006h,07fh,076h,005h,0c7h,046h,006h,07fh
    asm db 000h,08ah,046h,006h,088h,084h,050h,0cah,056h,0e8h,0dfh,006h,059h,05eh,05dh,0c3h
    asm _f_dad7_end label byte
    asm public _f_dad7_end
}
