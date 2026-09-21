/* F_C914 -- decode one music-stream command.  The DI/ES state and the
   historical short-jump backfills are preserved as inline bytes/calls. */
extern void sound_note_dispatch();
extern void sound_stream_delay_decode();
extern void speaker_gate_off();
extern void sound_ctlblock_command_dispatch();
extern void stream_note_program();
extern void stream_control_block_arm();
extern void sound_stop_reset();
extern unsigned int mus_ptr,mus_arg,mus_flag,snd_delay;

void sound_stream_command_step()
{
    asm db 055h,08Bh,0ECh,08Bh,03Eh
    asm dw offset DGROUP:mus_ptr
    asm db 026h,08Ah,005h,08Ah,0E0h,0D0h,0ECh,0D0h,0ECh,0D0h,0ECh,0D0h,0ECh,024h,00Fh,03Ch,000h,074h,015h,03Ch,00Dh,074h,01Ah,03Ch,00Eh,074h,021h,03Ch,00Fh,074h,023h
    asm call near ptr sound_note_dispatch
    asm call near ptr sound_stream_delay_decode
    asm db 0EBh,038h,090h
    asm call near ptr speaker_gate_off
    asm call near ptr sound_stream_delay_decode
    asm db 0EBh,02Fh,090h
    asm call near ptr sound_ctlblock_command_dispatch
    asm db 083h,03Eh
    asm dw offset DGROUP:snd_delay
    asm db 000h
    asm db 0EBh,024h,090h
    asm call near ptr stream_note_program
    asm db 0EBh,01Eh,090h
    asm call near ptr speaker_gate_off
    asm db 083h,03Eh
    asm dw offset DGROUP:mus_flag
    asm db 000h,074h,00Eh,0A1h
    asm dw offset DGROUP:mus_arg
    asm db 050h
    asm call near ptr stream_control_block_arm
    asm db 058h,0FFh,006h
    asm dw offset DGROUP:snd_delay
    asm db 05Dh,0C3h
    asm call near ptr sound_stop_reset
    asm db 05Dh,0C3h,083h,006h
    asm dw offset DGROUP:mus_ptr
    asm db 002h,083h,03Eh
    asm dw offset DGROUP:snd_delay
    asm db 000h,074h,091h,05Dh
}
