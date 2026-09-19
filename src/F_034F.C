/* F_034F -- switch to the text video mode through BIOS INT 10h. */
void f034f()
{
    asm mov ax,3
    asm int 10h
}
