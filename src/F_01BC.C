/* F_01BC -- load a 256-entry DAC block through int 10h AX=1012h.  A TC frame
   with a hand-written body (capsule rule 14's positive case): the frame is
   forced by the parameter, and the body stays out of SI and DI, so TC saves
   neither. */
void f01bc(pal)
char far *pal;
{
    asm les dx,pal
    asm xor bx,bx
    asm mov cx,100h
    asm mov ax,1012h
    asm int 10h
}
