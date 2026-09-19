/* F_C1A0 -- run the master sound tick.  The register-save sequence is
   byte-coded to retain the historical hand-written order. */
extern void fc1f7();
extern void fc8e2();
extern unsigned int v_b[],snd_flag,snd_mode,snd_paused,snd_seg2;

void fc1a0()
{
    asm db 055h,08Bh,0ECh,050h,051h,052h,053h,056h,057h,006h,033h,0C0h,0A3h
    asm dw offset DGROUP:v_b
    asm db 0A3h
    asm dw offset DGROUP:v_b+2
    asm db 0A3h
    asm dw offset DGROUP:v_b+4
    asm db 0A3h
    asm dw offset DGROUP:v_b+6
    asm db 083h,03Eh
    asm dw offset DGROUP:snd_flag
    asm db 000h,074h,021h
    asm call near ptr fc8e2
    asm db 083h,03Eh
    asm dw offset DGROUP:snd_mode
    asm db 000h,074h,025h,083h,03Eh
    asm dw offset DGROUP:snd_paused
    asm db 000h,075h,006h,0C7h,006h
    asm dw offset DGROUP:v_b
    asm db 001h,000h,08Eh,006h
    asm dw offset DGROUP:snd_seg2
    asm call near ptr fc1f7
    asm db 0EBh,00Fh,090h,083h,03Eh
    asm dw offset DGROUP:snd_mode
    asm db 000h,074h,007h,08Eh,006h
    asm dw offset DGROUP:snd_seg2
    asm call near ptr fc1f7
    asm db 007h,05Fh,05Eh,05Bh,05Ah,059h,058h,05Dh
}
