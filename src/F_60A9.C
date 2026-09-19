extern unsigned int g8fe, ga20;
extern unsigned char far *gbfc8, *g40d0;
extern void f03b4(), f03cc();

void f60a9()
{
    asm db 055h,08Bh,0ECh,056h,057h,01Eh,0FCh
    asm db 083h,03Eh
    asm dw offset DGROUP:g8fe
    asm db 0
    asm db 074h,003h,0E9h,0C2h,000h
    asm db 0FFh,00Eh
    asm dw offset DGROUP:ga20
    asm db 083h,03Eh
    asm dw offset DGROUP:ga20
    asm db 0
    asm db 074h,003h,0E9h,0B4h,000h
    asm db 0C7h,006h
    asm dw offset DGROUP:ga20
    asm db 00Ah,000h
    asm db 0C4h,03Eh
    asm dw offset DGROUP:gbfc8
    asm db 0C5h,036h
    asm dw offset DGROUP:g40d0
    asm db 0ACh,033h,0C9h,08Ah,0C8h,057h,0ADh,033h,0D2h,08Ah,0D4h,08Ah,0FEh,08Ah,0D8h,0D1h,0E3h,08Bh,0EAh,0ACh,032h,0E4h,0A8h,080h
    asm db 075h,003h,0E9h,082h,000h,0A8h,040h,074h,00Ah,024h,01Fh,0FEh,0C8h,07Dh,002h,0B0h,017h,0EBh,00Ah,024h,01Fh,0FEh,0C0h,03Ch
    asm db 017h,07Eh,002h,0B0h,000h,080h,064h,0FFh,0E0h,008h,044h,0FFh,0BAh,0E6h,001h,0F7h,0E2h,005h,002h,000h,003h,0F8h,08Bh,0D5h
    asm db 01Eh,051h,006h,052h
    asm mov ax,DGROUP
    asm db 08Eh,0D8h,052h,08Bh,0ECh,053h,0B8h,01Eh,000h,050h,0B8h,01Eh,000h,050h,08Bh,0C2h,005h,048h,001h,050h,053h
    asm call near ptr f03b4
    asm db 081h,046h,000h,0B8h,000h
    asm call near ptr f03b4
    asm db 05Bh,083h,0C4h,00Ah,05Ah,007h,006h,033h,0C0h,050h,006h,057h,052h,053h
    asm call near ptr f03cc
    asm db 05Bh,05Ah,083h,0C4h,006h,08Bh,0C2h,005h,0B8h,000h,050h,053h,0B8h,01Eh,000h,050h,0B8h,01Eh,000h,050h,052h,053h
    asm call near ptr f03b4
    asm db 083h,0C4h,00Ch,007h,059h,01Fh,05Fh,049h,0E3h,003h,0E9h,05Fh,0FFh,01Fh,05Fh,05Eh,05Dh
}
