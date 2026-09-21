/* F_6B7A -- install the timer interrupt and program the PIT divisor. */
extern unsigned int8_saved_vector;
extern unsigned g0b7c;
void timer_irq_install()
{
    asm push ax
    asm push dx
    asm push ds
    asm push es
    asm mov ax,3508h
    asm int 21h
    asm mov ax,es
    asm mov g0b7c,ax
    asm mov int8_saved_vector,bx
    asm mov dx,6bcfh
    asm push cs
    asm pop ds
    asm mov ax,2508h
    asm int 21h
    asm mov al,36h
    asm out 43h,al
    asm mov ax,13b1h
    asm out 40h,al
    asm mov al,ah
    asm out 40h,al
    asm mov al,0b6h
    asm out 43h,al
    asm pop es
    asm pop ds
    asm pop dx
    asm pop ax
}
