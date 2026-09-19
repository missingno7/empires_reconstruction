/* DOS_STUB -- exact startup stub and interrupt helper table. */
void dosstub()
{
    asm db 0bah
    asm dw _TEXT+0fa3h
    asm db 02eh,089h,016h,0bah,001h,0b4h,030h,0cdh,021h,08bh,02eh,002h,000h,08bh,01eh,02ch,000h,08eh,0dah,0a3h,07dh,000h,08ch,006h
    asm db 07bh,000h,089h,01eh,077h,000h,089h,02eh,091h,000h,0c7h,006h,081h,000h,0ffh,0ffh,0e8h,0f6h,000h,0c4h,03eh,075h,000h,08bh
    asm db 0c7h,08bh,0d8h,0b9h,0ffh,07fh,026h,081h,03dh,038h,037h,075h,019h,026h,08bh,055h,002h,080h,0fah,03dh,075h,010h,080h,0e6h
    asm db 0dfh,0ffh,006h,081h,000h,080h,0feh,059h,075h,004h,0ffh,006h,081h,000h,0f2h,0aeh,0e3h,03ch,043h,026h,038h,005h,075h,0d6h
    asm db 080h,0cdh,080h,0f7h,0d9h,089h,00eh,075h,000h,0b9h,002h,000h,0d3h,0e3h,083h,0c3h,010h,083h,0e3h,0f0h,089h,01eh,079h,000h
    asm db 08ch,0d2h,02bh,0eah,08bh,03eh,0b8h,037h,081h,0ffh,000h,002h,073h,007h,0bfh,000h,002h,089h,03eh,0b8h,037h,0b1h,004h,0d3h
    asm db 0efh,047h,03bh,0efh,073h,003h,0e9h,008h,001h,08bh,0dfh,003h,0dah,089h,01eh,089h,000h,089h,01eh,08dh,000h,0a1h,07bh,000h
    asm db 02bh,0d8h,08eh,0c0h,0b4h,04ah,057h,0cdh,021h,05fh,0d3h,0e7h,0fah,08eh,0d2h,08bh,0e7h,0fbh,033h,0c0h,02eh,08eh,006h,0bah
    asm db 001h,0bfh,002h,039h,0b9h,0c8h,0cah,02bh,0cfh,0f3h,0aah,00eh,0ffh,016h,0fch,038h,0e8h,086h,0e5h,0e8h,07ah,0e6h,0b4h,000h
    asm db 0cdh,01ah,089h,016h,083h,000h,089h,00eh,085h,000h,0ffh,016h,000h,039h,0ffh,036h,073h,000h,0ffh,036h,071h,000h,0ffh,036h
    asm db 06fh,000h,0ffh,036h,06dh,000h,0ffh,036h,06bh,000h,0e8h,093h,049h,050h,0e8h,023h,0e5h,02eh,08eh,01eh,0bah,001h,0e8h,05bh
    asm db 000h,00eh,0ffh,016h,0feh,038h,08bh,0ech,0b4h,04ch,08ah,046h,002h,0cdh,021h,0b9h,00eh,000h,090h,0bah,02fh,000h,0e9h,087h
    asm db 000h,01eh,0b8h,000h,035h,0cdh,021h,089h,01eh,05bh,000h,08ch,006h,05dh,000h,0b8h,004h,035h,0cdh,021h,089h,01eh,05fh,000h
    asm db 08ch,006h,061h,000h,0b8h,005h,035h,0cdh,021h,089h,01eh,063h,000h,08ch,006h,065h,000h,0b8h,006h,035h,0cdh,021h,089h,01eh
    asm db 067h,000h,08ch,006h,069h,000h,0b8h,000h,025h,08ch,0cah,08eh,0dah,0bah,01ah,001h,0cdh,021h,01fh,0c3h,01eh,0b8h,000h,025h
    asm db 0c5h,016h,05bh,000h,0cdh,021h,01fh,01eh,0b8h,004h,025h,0c5h,016h,05fh,000h,0cdh,021h,01fh,01eh,0b8h,005h,025h,0c5h,016h
    asm db 063h,000h,0cdh,021h,01fh,01eh,0b8h,006h,025h,0c5h,016h,067h,000h,0cdh,021h,01fh,0c3h
    asm _dosstub_end label byte
    asm public _dosstub_end
}
