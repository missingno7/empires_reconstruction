/* F_C27D -- service each sound voice.  SI is intentionally emitted as raw
   bytes so Turbo C does not add a save pair absent from the original loop. */
extern void sound_command_stream_dispatch();
extern void voice_disable();
extern void sound_voices_reset_and_service();
extern void sound_control_block_advance();
extern unsigned int v_a[];
extern unsigned int v_ctr;
extern unsigned int snd_backend_mode;
extern unsigned int v_hold;
extern unsigned int v_len;
extern unsigned int snd_nvoices;

void sound_voice_service_loop()
{
    asm db 055h,08Bh,0ECh,033h,0F6h
    asm db 083h,0BCh
    asm dw offset DGROUP:v_a
    asm db 000h,074h,031h
    asm db 083h,0BCh
    asm dw offset DGROUP:v_ctr
    asm db 000h,075h,00Ah
    asm call near ptr sound_command_stream_dispatch
    asm db 083h,0BCh
    asm dw offset DGROUP:v_a
    asm db 000h,074h,020h
    asm db 0FFh,08Ch
    asm dw offset DGROUP:v_ctr
    asm db 083h,03Eh
    asm dw offset DGROUP:snd_backend_mode
    asm db 001h,074h,015h
    asm db 083h,0BCh
    asm dw offset DGROUP:v_hold
    asm db 001h,074h,00Eh
    asm db 08Bh,084h
    asm dw offset DGROUP:v_len
    asm db 048h,039h,084h
    asm dw offset DGROUP:v_ctr
    asm db 075h,003h
    asm call near ptr voice_disable
    asm db 083h,0C6h,002h,08Bh,0C6h,0D1h,0E8h,03Bh,006h
    asm dw offset DGROUP:snd_nvoices
    asm db 07Ch,0BBh,0A1h
    asm dw offset DGROUP:v_a
    asm db 00Bh,006h
    asm dw offset DGROUP:v_a+2
    asm db 00Bh,006h
    asm dw offset DGROUP:v_a+4
    asm db 00Bh,006h
    asm dw offset DGROUP:v_a+6
    asm db 075h,006h
    asm call near ptr sound_voices_reset_and_service
    asm db 0EBh,00Bh,090h,083h,03Eh
    asm dw offset DGROUP:snd_backend_mode
    asm db 001h,075h,003h
    asm call near ptr sound_control_block_advance
    asm db 05Dh
}
