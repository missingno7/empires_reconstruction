/* F_8C04 -- continuation and epilogue extent. */
void f_8c04()
{
    asm db 0e8h,04ch,0fah,083h,07eh,0f8h,000h,074h,003h,0e8h,00eh,0efh,0e8h,093h,04bh,0e8h,04ch,0e7h,083h,07eh,0f6h,000h,075h,003h
    asm db 0e8h,078h,0dfh,0e8h,044h,0e1h,00bh,0f6h,07dh,006h,033h,0c0h,0ebh,007h,0ebh,005h,0b8h,001h,000h,0ebh,000h,05fh,05eh,08bh
    asm db 0e5h,05dh,0c3h
    asm _f_8c04_end label byte
    asm public _f_8c04_end
}
