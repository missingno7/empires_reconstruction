extern unsigned int gc0d4,g0b76,g0b78,g176e,g1772,g237c;
extern unsigned long g0b7a;
extern void fc1a0();

void interrupt f6bcf()
{
    asm db 09Ch,0FBh
    asm db 0FFh,006h
    asm dw offset DGROUP:gc0d4
    asm db 083h,03Eh
    asm dw offset DGROUP:gc0d4
    asm db 00Dh
    asm db 07Ch,00Bh
    asm db 0C7h,006h
    asm dw offset DGROUP:gc0d4
    asm db 000h,000h
    asm db 09Ch
    asm db 0FFh,01Eh
    asm dw offset DGROUP:g0b7a
    asm db 083h,006h
    asm dw offset DGROUP:g0b76
    asm db 001h
    asm db 083h,016h
    asm dw offset DGROUP:g0b78
    asm db 000h
    asm db 083h,03Eh
    asm dw offset DGROUP:g237c
    asm db 000h
    asm db 075h,011h
    asm db 083h,03Eh
    asm dw offset DGROUP:g176e
    asm db 000h
    asm db 075h,007h
    asm db 083h,03Eh
    asm dw offset DGROUP:g1772
    asm db 000h
    asm db 074h,003h
    asm call near ptr fc1a0
    asm db 0B0h,020h,0E6h,020h,09Dh
}
