/* F_C8D4 -- write the caller's live AL to the configured I/O port. */
extern unsigned g1830;
void fc8d4()
{
    asm db 055h,08bh,0ech
    asm push dx
    asm mov dx,g1830
    asm out dx,al
    asm db 0ebh,0
    asm pop dx
    asm db 05dh
}
