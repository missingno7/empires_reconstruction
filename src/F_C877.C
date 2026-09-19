/* F_C877 -- pump one timer slice through the owned F_6B9 routine. */
extern unsigned g1778,g177a;
extern void fc6b9();
void fc877(dummy)
int dummy;
{
    asm db 051h,056h,033h,0f6h
    asm db 08bh,00eh
    asm dw g177a
    asm db 083h,03eh
    asm dw g1778
    asm db 2,075h,3,0b9h,4,0
    asm call near ptr fc6b9
    asm db 083h,0c6h,2,0e2h,0f8h,05eh,059h
}
