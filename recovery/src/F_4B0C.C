/* F_4B0C -- exact non-returning interpreter loop boundary probe. */
void sprite_script_frame_driver()
{
    asm db 055h,08bh,0ech,056h,057h,055h,0fch,0b8h,00fh,000h,050h,0e8h,0b4h,0b6h,058h,0bfh,0aeh,0b3h,033h,0c9h,08ah,00dh,00bh,0c9h
    asm db 074h,045h,047h,051h,033h,0c0h,08ah,045h,001h,03bh,006h,0bah,0bfh,075h,032h,08ah,005h,03ch,001h,074h,03ah,080h,07dh,00ah
    asm db 000h,075h,031h,08bh,075h,00dh,081h,0c6h,0aeh,0b3h,033h,0dbh,0ach,08ah,0d8h,0d1h,0e3h,081h,0c3h,04ah,007h,0ffh,027h,081h
    asm db 0eeh,0aeh,0b3h,089h,075h,00dh,033h,0c0h,08ah,045h,001h,03bh,006h,0bah,0bfh,074h,033h,059h,083h,0c7h,020h,0e2h,0bch,05dh
    asm db 05fh,05eh,05dh,0c3h,0feh,04dh,00ah,080h,07dh,008h,001h,074h,0ech,033h,0dbh,08ah,05dh,007h,053h,08ah,05dh,006h,0d1h,0e3h
    asm db 0d1h,0e3h,0c4h,087h,07ch,0b0h,006h,050h,0ffh,075h,004h,0ffh,075h,002h,0e8h,037h,0b8h,083h,0c4h,00ah,080h,07dh,008h,001h
    asm db 074h,0c7h,080h,07dh,01ah,000h,074h,01ah,08bh,045h,002h,005h,010h,000h,08bh,05dh,004h,08ah,055h,01ah,032h,0f6h,02bh,0dah
    asm db 043h,053h,052h,050h,0e8h,0eah,0b7h,083h,0c4h,006h,033h,0dbh,08ah,05dh,006h,0d1h,0e3h,002h,05dh,007h,0d1h,0e3h,0d1h,0e3h
    asm db 08bh,087h,0d4h,040h,083h,0c3h,002h,08bh,08fh,0d4h,040h,033h,0d2h,086h,0d5h,08bh,0eah,08ah,0d4h,032h,0e4h,08bh,05dh,002h
    asm db 003h,0c3h,003h,0cbh,08bh,05dh,004h,003h,0d3h,003h,0ebh,08bh,01eh,02eh,007h,0d1h,0e3h,003h,01eh,03ah,007h,0d1h,0e3h,0d1h
    asm db 0e3h,056h,057h,08bh,0f3h,081h,0c6h,09eh,007h,08bh,0f8h,0adh,033h,0dbh,086h,0dch,003h,006h,036h,007h,03bh,0c1h,07fh,062h
    asm db 003h,01eh,038h,007h,03bh,0ddh,07fh,05ah,0adh,033h,0dbh,086h,0dch,003h,006h,036h,007h,03bh,0f8h,07fh,04dh,003h,01eh,038h
    asm db 007h,03bh,0d3h,07fh,045h,08bh,0c7h,05fh,05eh,083h,03eh,0ceh,040h,000h,075h,006h,05bh,053h,089h,01eh,0ceh,040h,08ah,05dh
    asm db 019h,00ah,0dbh,074h,031h,080h,07dh,01bh,001h,074h,02bh,080h,0fbh,001h,075h,007h,080h,03eh,02ch,007h,000h,074h,01fh,033h
    asm db 0dbh,089h,05dh,00fh,089h,05dh,011h,089h,05dh,013h,089h,05dh,015h,0feh,0c3h,088h,05dh,01bh,08bh,075h,017h,089h,075h,00dh
    asm db 0ebh,004h,08bh,0c7h,05fh,05eh,080h,03eh,0feh,008h,000h,074h,02eh,080h,07dh,00ah,000h,075h,028h,08bh,01eh,04eh,0c0h,0d1h
    asm db 0e3h,053h,08bh,09fh,050h,0c0h,03bh,0d8h,058h,07ch,018h,03bh,0d9h,07fh,014h,08bh,0d8h,08bh,087h,080h,0c0h,03bh,0c2h,07ch
    asm db 00ah,03bh,0c5h,07fh,006h,08ah,045h,009h,088h,045h,00ah,0e9h,0b3h,0feh,081h,0eeh,0aeh,0b3h,089h,075h,00dh,0e9h,0b7h,0feh
    asm db 0adh,003h,0f0h,0e9h,084h,0feh,0adh,089h,075h,00fh,003h,0f0h,0e9h,07bh,0feh,08bh,075h,00fh,0e9h,075h,0feh,0bbh,011h,000h
    asm db 0ebh,008h,0bbh,013h,000h,0ebh,003h,0bbh,015h,000h,0adh,08bh,0c8h,0adh,0ffh,009h,074h,006h,079h,002h,089h,001h,003h,0f1h
    asm db 0e9h,057h,0feh,032h,0e4h,0ach,050h,0e8h,0fbh,07dh,058h,0e9h,04ch,0feh,032h,0e4h,0ach,050h,0e8h,02ch,0ddh,059h,052h,050h
    asm db 0e8h,083h,0e6h,059h,059h,0e9h,03ah,0feh,032h,0e4h,0ach,050h,0e8h,0ddh,0e9h,058h,0e9h,02fh,0feh,0b2h,001h,0ebh,002h,0b2h
    asm db 000h,032h,0e4h,0ach,03ch,080h,073h,015h,08bh,0d8h,0d1h,0e3h,0d1h,0e3h,0d1h,0e3h,0d1h,0e3h,0d1h,0e3h,081h,0c3h,0afh,0b3h
    asm db 088h,017h,0e9h,00dh,0feh,024h,07fh,08bh,0d8h,0d1h,0e3h,0d1h,0e3h,0d1h,0e3h,0d1h,0e3h,0d1h,0e3h,003h,0dfh,088h,017h,0e9h
    asm db 0f8h,0fdh,0adh,088h,045h,00bh,088h,065h,00ch,0e9h,0eeh,0fdh,033h,0c0h,0ach,0d1h,0e0h,0d0h,0e8h,088h,045h,006h,088h,065h
    asm db 007h,0e9h,0deh,0fdh,08bh,055h,002h,08bh,04dh,004h,0adh,08ah,0dch,098h,003h,0d0h,08ah,0c3h,098h,003h,0c8h,089h,055h,002h
    asm db 089h,04dh,004h,033h,0c0h,0ach,0d1h,0e0h,0d0h,0e8h,088h,065h,007h,002h,045h,006h,03ah,045h,00bh,07ch,02ah,03ah,045h,00ch
    asm db 07fh,02ah,088h,045h,006h,080h,07dh,008h,001h,074h,019h,033h,0dbh,08ah,0dch,053h,08ah,0d8h,0d1h,0e3h,0d1h,0e3h,0c4h,087h
    asm db 07ch,0b0h,006h,050h,051h,052h,0e8h,017h,0b6h,083h,0c4h,00ah,0e9h,098h,0fdh,08ah,045h,00ch,0ebh,0d6h,08ah,045h,00bh,0ebh
    asm db 0d1h,0adh,033h,0c9h,08bh,0d1h,08ah,0d0h,08ah,0cch,0d1h,0e2h,089h,055h,002h,089h,04dh,004h,033h,0c0h,0ach,0d1h,0e0h,0d0h
    asm db 0e8h,088h,065h,007h,0ebh,0b4h,0adh,033h,0c9h,08bh,0d1h,08ah,0d0h,08ah,0cch,0d1h,0e2h,089h,055h,002h,089h,04dh,004h,0adh
    asm db 088h,065h,001h,033h,0dbh,086h,0dch,0d1h,0e0h,0d0h,0e8h,088h,065h,007h,03bh,01eh,0bah,0bfh,074h,08eh,0ebh,0aeh,0c6h,045h
    asm db 008h,001h,0c6h,045h,01bh,000h,0e9h,03eh,0fdh,0c6h,045h,008h,000h,0e9h,02ah,0fdh,0adh,08bh,0d8h,0f6h,087h,074h,043h,007h
    asm db 074h,05eh,0e9h,01dh,0fdh,0adh,08bh,0d8h,0f6h,087h,074h,043h,007h,075h,051h,0e9h,010h,0fdh,0adh,08bh,0d8h,0f6h,087h,074h
    asm db 043h,010h,075h,044h,0e9h,003h,0fdh,0adh,08bh,0d8h,0f6h,087h,074h,043h,010h,074h,037h,0e9h,0f6h,0fch,033h,0c0h,0ach,0d1h
    asm db 0e0h,039h,006h,036h,007h,07eh,029h,0e9h,0e8h,0fch,033h,0c0h,0ach,0d1h,0e0h,039h,006h,036h,007h,073h,01bh,0e9h,0dah,0fch
    asm db 033h,0c0h,0ach,039h,006h,038h,007h,07eh,00fh,0e9h,0ceh,0fch,033h,0c0h,0ach,039h,006h,038h,007h,073h,003h,0e9h,0c2h,0fch
    asm db 033h,0dbh,0ach,08ah,0d8h,08ah,09fh,082h,007h,003h,0f3h,0e9h,0b4h,0fch,0e8h,07ah,0aah,08bh,0d8h,0ach,03ah,0d8h,076h,0e8h
    asm db 0e9h,0a7h,0fch
    asm _f4b0c_end label byte
    asm public _f4b0c_end
}
