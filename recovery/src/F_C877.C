/* F_C877 -- pump one timer slice through the owned F_6B9 routine. */
extern unsigned snd_backend_mode,g177a;
extern void voice_disable();
void sound_voices_disable_all(dummy)
int dummy;
{
    asm db 051h,056h,033h,0f6h
    asm db 08bh,00eh
    asm dw g177a
    asm db 083h,03eh
    asm dw snd_backend_mode
    asm db 2,075h,3,0b9h,4,0
    asm call near ptr voice_disable
    asm db 083h,0c6h,2,0e2h,0f8h,05eh,059h
}
