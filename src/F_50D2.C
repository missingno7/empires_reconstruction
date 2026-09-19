/* F_50D2 -- display-adapter probe.  The historical hardware sequence and
   assembler backfill bytes are retained as deterministic inline data. */
extern unsigned int bbfcd;
extern unsigned int b856;

void f50d2()
{
    asm db 056h,0C6h,006h
    asm dw offset DGROUP:bbfcd
    asm db 000h,090h,0B4h,00Fh,0CDh,010h,03Ch,007h,075h,003h,0E9h,0AEh,000h,0B8h,000h,0F0h,08Eh,0C0h,0BEh,0FEh,0FFh,026h,080h,03Ch
    asm db 0FFh,075h,017h,0BEh,000h,0C0h,026h,080h,03Ch,021h,075h,00Eh,0C6h,006h
    asm dw offset DGROUP:bbfcd
    asm db 003h,090h,0C6h,006h
    asm dw offset DGROUP:b856
    asm db 001h,0E9h,089h,000h,0B8h,000h,01Ah,0CDh,010h,03Ch,01Ah,075h,042h,080h,0FBh,00Ah,07Ch,009h,0C6h,006h
    asm dw offset DGROUP:bbfcd
    asm db 004h,090h,0EBh,073h,090h,080h,0FBh,004h,074h,005h,080h,0FBh,005h,075h,009h,0C6h,006h
    asm dw offset DGROUP:bbfcd
    asm db 001h,090h,0EBh,060h,090h,080h,0FBh,007h,074h,005h,080h,0FBh,008h,075h,009h,0C6h,006h
    asm dw offset DGROUP:bbfcd
    asm db 005h,090h,0EBh,04Dh,090h,080h,0FBh,002h,075h,009h,0C6h,006h
    asm dw offset DGROUP:bbfcd
    asm db 002h,090h,0EBh,03Fh,090h,0B7h,0FFh,0B1h,0FFh,0B4h,012h,0B3h,010h,0CDh,010h,080h,0FFh,001h,077h,00Eh,080h,0F9h,00Fh,077h
    asm db 009h,0C6h,006h
    asm dw offset DGROUP:bbfcd
    asm db 001h,090h,0EBh,022h,090h,0BAh,0D4h,003h,0B0h,00Fh,0EEh,042h,0ECh,08Ah,0E0h,0B0h,066h,0EEh,0B9h,000h,001h,0E2h,0FEh,0ECh
    asm db 086h,0E0h,0EEh,080h,0FCh,066h,075h,006h,0C6h,006h
    asm dw offset DGROUP:bbfcd
    asm db 002h,090h,083h,03Eh
    asm dw offset DGROUP:bbfcd
    asm db 001h,075h,013h,0B4h,012h,0B3h,010h,0CDh,010h,00Ah,0DBh,075h,01Ah,0C6h,006h
    asm dw offset DGROUP:bbfcd
    asm db 002h,090h,0EBh,012h,090h,083h,03Eh
    asm dw offset DGROUP:bbfcd
    asm db 004h,075h,003h,0EBh,008h,090h,083h,03Eh
    asm dw offset DGROUP:bbfcd
    asm db 005h,075h,000h,05Eh
}
