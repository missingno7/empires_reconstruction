extern unsigned int gb68,gb6a,gb6c,gb6e,gb70,gb72;
extern unsigned long gc0cc;
extern void f68cf();

void interrupt f699e()
{
    asm db 08Bh,0ECh,083h,0ECh,004h,0FBh,0E4h,060h,088h,046h,0FFh,0BFh,001h,000h,080h,07Eh,0FFh,0E0h,074h,006h,080h,07Eh,0FFh,0E1h
    asm db 075h,01Ah,0E4h,061h,0B4h,000h,089h,046h,0FCh,08Ah,046h,0FCh,00Ch,080h,0E6h,061h,08Ah,046h,0FCh,0E6h,061h,0B0h,020h,0E6h
    asm db 020h,0E9h,02Eh,001h,080h,07Eh,0FFh,080h,073h,005h,0BEh,001h,000h,0EBh,002h,033h,0F6h,08Ah,046h,0FFh,0B4h,000h,025h,07Fh
    asm db 000h,0B9h,00Fh,000h,0BBh,00Bh,06Ah,02Eh,039h,007h,074h,007h,043h,043h,0E2h,0F7h,0E9h,0DAh,000h,02Eh,0FFh,067h,01Eh,01Dh
    asm db 000h,01Fh,000h,029h,000h,02Bh,000h,046h,000h,047h,000h,048h,000h,049h,000h,04Ah,000h,04Bh,000h,04Dh,000h,04Eh,000h,050h
    asm db 000h,054h,000h,058h,000h,0DDh,06Ah,0C9h,06Ah,07Fh,06Ah,09Ah,06Ah,0C7h,06Ah,051h,06Ah,088h,06Ah,068h,06Ah,0B8h,06Ah,0A3h
    asm db 06Ah,0B2h,06Ah,0A9h,06Ah,0C1h,06Ah,0C7h,06Ah,047h,06Ah,080h,03Eh,056h,008h,000h,075h,003h,0E9h,094h,000h,08Bh,0C6h
    asm db 0A3h
    asm dw offset DGROUP:gb6c
    asm db 0A3h
    asm dw offset DGROUP:gb68
    asm db 0F6h,046h,0FFh,080h,074h,006h
    asm db 0C7h,006h
    asm dw offset DGROUP:gb70
    asm db 001h,000h
    asm db 0EBh,07Eh,090h,08Bh,0C6h,0A3h,06Eh,00Bh,0A3h,068h,00Bh,0F6h,046h,0FFh,080h,074h,006h,0C7h,006h,070h,00Bh,001h,000h,0EBh
    asm db 067h,090h,080h,03Eh,056h,008h,000h,075h,002h,0EBh,05Dh
    asm db 089h,036h
    asm dw offset DGROUP:gb68
    asm db 0F6h,046h,0FFh,080h,074h,006h,0C7h,006h,070h,00Bh,001h,000h,0EBh,04Bh,080h,03Eh,056h,008h,000h,075h,002h,0EBh,042h
    asm db 089h,036h
    asm dw offset DGROUP:gb6c
    asm db 0EBh,03Ch,080h,03Eh,056h,008h,000h,075h,002h,0EBh,033h
    asm db 089h,036h
    asm dw offset DGROUP:gb6e
    asm db 0EBh,02Dh,080h,03Eh,056h,008h,000h,075h,002h,0EBh,024h
    asm db 089h,036h
    asm dw offset DGROUP:gb6a
    asm db 0EBh,01Eh,0EBh,01Ch,083h,03Eh,074h,00Bh,000h,074h,009h,00Bh,0F6h,074h,005h
    asm call near ptr f68cf
    asm db 0EBh,002h,033h,0FFh,0EBh,008h,089h,036h,074h,00Bh,033h,0FFh,0EBh,000h,00Bh,0FFh,074h,007h
    asm db 083h,03Eh
    asm dw offset DGROUP:gb72
    asm db 000h
    asm db 074h,007h,09Ch
    asm db 0FFh,01Eh
    asm dw offset DGROUP:gc0cc
    asm db 0EBh,017h,0E4h,061h,0B4h,000h,089h,046h,0FCh,08Ah,046h,0FCh,00Ch,080h,0E6h,061h,08Ah,046h,0FCh,0E6h,061h,0B0h,020h,0E6h
    asm db 020h
    asm db 08Bh,0E5h
}
