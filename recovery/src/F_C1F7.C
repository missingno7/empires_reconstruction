/* F_C1F7 -- service the sound voices and copy the pending voice state. */
extern void sound_voice_table_prime();
extern void sound_voice_service_loop();
extern unsigned int snd_mode;
extern unsigned int snd_flag2;
extern unsigned int sv[];
extern unsigned int dv[];

void sound_voice_pump_loop()
{
    asm push bp
    asm mov bp,sp
    asm cmp word ptr snd_mode,2
    asm je L2
L1: asm call near ptr sound_voice_table_prime
L2: asm call near ptr sound_voice_service_loop
    asm cmp word ptr snd_mode,0
    asm jne L3
    asm mov ax,word ptr snd_flag2
    asm cmp ax,0
    asm je L3
    asm mov ax,word ptr sv
    asm mov word ptr dv,ax
    asm mov ax,word ptr sv+2
    asm mov word ptr dv+2,ax
    asm mov ax,word ptr sv+4
    asm mov word ptr dv+4,ax
    asm mov ax,word ptr sv+6
    asm mov word ptr dv+6,ax
    asm jmp L1
L3: asm pop bp
}
