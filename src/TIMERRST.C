/* F_6BAC -- restore the timer interrupt and reset the PIT divisor. */
extern void interrupt (*int8_saved_vector)(void);   /* saved INT 8 vector: offset at DS:0B7A, segment at DS:0B7C */
void timer_irq_restore()
{
    asm push ax
    asm push dx
    asm push ds
    asm push es
    asm mov dx,word ptr int8_saved_vector
    asm mov ax,word ptr int8_saved_vector+2
    asm mov ds,ax
    asm mov ax,2508h
    asm int 21h
    asm mov al,36h
    asm out 43h,al
    asm xor ax,ax
    asm out 40h,al
    asm mov al,ah
    asm out 40h,al
    asm pop es
    asm pop ds
    asm pop dx
    asm pop ax
}
