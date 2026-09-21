/* F_C8E2 -- sound tick step.  Loads ES from a global (no C expression emits
   mov es,[mem]), so the body is asm; frame from the module's -k. */
/*@FLAGS -k*/
extern unsigned int snd_seg;            /* DS:1760 */
/*@SYM _snd_seg=0x1760 kind=g key=storage_objects/G_P21290.phys*/
extern int snd_on;                      /* DS:1770 */
/*@SYM _snd_on=0x1770 kind=g key=storage_objects/G_P212A0.phys*/
extern int snd_delay;                   /* DS:1E90 */
/*@SYM _snd_delay=0x1E90 kind=g key=storage_objects/G_P219C0.phys*/
extern int snd_one;                     /* DS:1E94 */
/*@SYM _snd_one=0x1E94 kind=g key=storage_objects/G_P219C4.phys*/
extern int snd_len;                     /* DS:1E8E */
/*@SYM _snd_len=0x1E8E kind=g key=storage_objects/G_P219BE.phys*/
extern void fc914();
extern void speaker_gate_off();

void sound_tick_step()
{
    asm mov es,word ptr snd_seg
    asm cmp word ptr snd_delay,0
    asm jne L1
    asm call near ptr fc914
    asm cmp word ptr snd_on,0
    asm je  L2
L1: asm dec word ptr snd_delay
    asm cmp word ptr snd_one,1
    asm je  L2
    asm mov ax,word ptr snd_len
    asm dec ax
    asm cmp word ptr snd_delay,ax
    asm jne L2
    asm call near ptr speaker_gate_off
L2: ;
}
