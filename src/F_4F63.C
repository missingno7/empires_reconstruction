/* F_4F63 -- write a far, terminated buffer through DOS handle 2. */
extern unsigned strlen();

void f4f63()
{
    asm db 055h,08bh,0ech,083h,0ech,2,056h,057h
    asm db 0ffh,076h,06h,0ffh,076h,04h
    asm call near ptr strlen
    asm db 059h,059h,08bh,0f0h,04eh,08bh,07eh,04h
    asm db 08bh,046h,06h,089h,046h,0feh,0bbh,02h,0
    asm db 08bh,0ceh,08bh,0d7h,0b4h,040h,08eh,05eh,0feh
    asm db 0cdh,021h,05fh,05eh,08bh,0e5h,05dh
}
