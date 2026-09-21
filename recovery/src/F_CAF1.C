/* F_CAF1 -- update the sound tick state for one pending voice. */
extern unsigned int sound_enabled,snd_seg,g1e8a,snd_base,g1e88,snd_on,g1e8e,stream_note_delay,g1e86,g1e90,g1e94,g1e8c;

void stream_control_block_arm()
{
    asm db 055h,08Bh,0ECh,050h,051h,056h,057h,006h
    asm db 083h,03Eh
    asm dw offset DGROUP:sound_enabled
    asm db 000h,074h,041h,08Eh,006h
    asm dw offset DGROUP:snd_seg
    asm db 08Bh,07Eh,004h,03Bh,03Eh
    asm dw offset DGROUP:g1e8a
    asm db 077h,034h,089h,03Eh
    asm dw offset DGROUP:g1e8a
    asm db 0D1h,0E7h,003h,03Eh
    asm dw offset DGROUP:snd_base
    asm db 026h,08Bh,005h,003h,006h
    asm dw offset DGROUP:snd_base
    asm db 0A3h
    asm dw offset DGROUP:g1e88
    asm db 0C7h,006h
    asm dw offset DGROUP:snd_on
    asm db 002h,000h,0C7h,006h
    asm dw offset DGROUP:g1e8e
    asm db 006h,000h,0C7h,006h
    asm dw offset DGROUP:stream_note_delay
    asm db 001h,000h,033h,0C0h,0A3h
    asm dw offset DGROUP:g1e86
    asm db 0A3h
    asm dw offset DGROUP:g1e90
    asm db 0A3h
    asm dw offset DGROUP:g1e94
    asm db 0A3h
    asm dw offset DGROUP:g1e8c
    asm db 007h,05Fh,05Eh,059h,058h,05Dh
}
