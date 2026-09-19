/* F_C834 -- reset the sound voices.  Register saves and SI addressing are
   byte-coded to preserve the hand-written module's order exactly. */
extern void f_c6b9();
extern unsigned int snd_mode,snd_hi,snd_nvoices,snd_paused,v_a[],v_b[];

void fc834()
{
    asm db 055h,08Bh,0ECh,050h,051h,056h,0C7h,006h
    asm dw offset DGROUP:snd_mode
    asm db 000h,000h,0C7h,006h
    asm dw offset DGROUP:snd_hi
    asm db 0FFh,0FFh,033h,0F6h,08Bh,00Eh
    asm dw offset DGROUP:snd_nvoices
    asm db 051h,083h,03Eh
    asm dw offset DGROUP:snd_paused
    asm db 002h,075h,006h,0C7h,006h
    asm dw offset DGROUP:snd_nvoices
    asm db 004h,000h
    asm db 0C7h,084h
    asm dw offset DGROUP:v_a
    asm db 000h,000h,0C7h,084h
    asm dw offset DGROUP:v_b
    asm db 000h,000h
    asm call near ptr f_c6b9
    asm db 083h,0C6h,002h,0E2h,0ECh,08Fh,006h
    asm dw offset DGROUP:snd_nvoices
    asm db 05Eh,059h,058h,05Dh
}
