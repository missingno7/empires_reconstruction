/* F_6CA6 -- index the far resource table and publish three spans. */
extern unsigned gc0d6[];
extern unsigned gc0de,gc0e0,gc0e2,gc0e4,gc0e6,gc0e8,gc0ea;
void f6ca6(i)
int i;
{
    asm db 056h
    asm mov bx,[bp+4]
    asm db 089h,01eh
    asm dw gc0ea
    asm db 0d1h,0e3h,0d1h,0e3h,0c4h,0b7h
    asm dw gc0d6
    asm db 08ch,0c0h,0a3h
    asm dw gc0e0
    asm db 033h,0c0h,08bh,0c8h,026h,08ah,04ch,1,041h,026h,08ah,044h,2,0a3h
    asm dw gc0e8
    asm db 083h,0c6h,3,089h,036h
    asm dw gc0e4
    asm db 03h,0f1h,089h,036h
    asm dw gc0e6
    asm db 03h,0f1h,089h,036h
    asm dw gc0e2
    asm db 03h,0f1h,089h,036h
    asm dw gc0de
    asm db 05eh
}

