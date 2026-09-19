/* F_C914 -- decode one music-stream command.  The DI/ES state and the
   historical short-jump backfills are preserved as inline bytes/calls. */
extern void fc988();
extern void fc9a4();
extern void fcadb();
extern void fca03();
extern void fca9b();
extern void fcaf1();
extern void fcb48();
extern unsigned int mus_ptr,mus_arg,mus_flag,snd_delay;

void fc914()
{
    asm db 055h,08Bh,0ECh,08Bh,03Eh
    asm dw offset DGROUP:mus_ptr
    asm db 026h,08Ah,005h,08Ah,0E0h,0D0h,0ECh,0D0h,0ECh,0D0h,0ECh,0D0h,0ECh,024h,00Fh,03Ch,000h,074h,015h,03Ch,00Dh,074h,01Ah,03Ch,00Eh,074h,021h,03Ch,00Fh,074h,023h
    asm call near ptr fc988
    asm call near ptr fc9a4
    asm db 0EBh,038h,090h
    asm call near ptr fcadb
    asm call near ptr fc9a4
    asm db 0EBh,02Fh,090h
    asm call near ptr fca03
    asm db 083h,03Eh
    asm dw offset DGROUP:snd_delay
    asm db 000h
    asm db 0EBh,024h,090h
    asm call near ptr fca9b
    asm db 0EBh,01Eh,090h
    asm call near ptr fcadb
    asm db 083h,03Eh
    asm dw offset DGROUP:mus_flag
    asm db 000h,074h,00Eh,0A1h
    asm dw offset DGROUP:mus_arg
    asm db 050h
    asm call near ptr fcaf1
    asm db 058h,0FFh,006h
    asm dw offset DGROUP:snd_delay
    asm db 05Dh,0C3h
    asm call near ptr fcb48
    asm db 05Dh,0C3h,083h,006h
    asm dw offset DGROUP:mus_ptr
    asm db 002h,083h,03Eh
    asm dw offset DGROUP:snd_delay
    asm db 000h,074h,091h,05Dh
}
