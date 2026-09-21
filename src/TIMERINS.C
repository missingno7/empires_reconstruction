/* F_6B7A -- install the timer interrupt and program the PIT divisor. */
extern void interrupt (*int8_saved_vector)(void);   /* saved INT 8 vector: offset at DS:0B7A, segment at DS:0B7C */
void timer_irq_install()
{
    asm push ax
    asm push dx
    asm push ds
    asm push es
    asm mov ax,3508h
    asm int 21h
    asm mov ax,es
    asm mov word ptr int8_saved_vector+2,ax
    asm mov word ptr int8_saved_vector,bx
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
