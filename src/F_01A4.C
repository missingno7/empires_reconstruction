/* F_01A4 -- DOS write setup and process handoff. */
extern void f019c();
extern void f0104();

void f01a4()
{
    asm mov cx,01eh
    asm nop
    asm mov dx,03dh
    asm mov ds,word ptr cs:01bah
    asm call near ptr f019c
    asm mov ax,3
    asm push ax
    asm call near ptr f0104
    asm _f01a4_end label byte
    asm public _f01a4_end
}
