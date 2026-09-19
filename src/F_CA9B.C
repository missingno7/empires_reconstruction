/* F_CA9B -- sound-note shift path; AH/AL, BX and ES:DI arrive live. */
extern unsigned g1e92, g1e90;
extern void fcadb();
extern void fcae6();
extern void fcad0();
void fca9b()
{
    asm db 055h,08bh,0ech
    asm db 026h,08ah,055h,1,032h,0f6h,083h,0fah,0
    asm db 075h,6
    asm call near ptr fcadb
    asm db 0ebh,01ah,090h,0b1h,7,0d3h,0e2h,0f7h,0dah
    asm db 08ah,0cch,033h,0dbh,0bbh,0fch,017h,08bh,07h
    asm db 03h,0c2h,0d3h,0e8h
    asm call near ptr fcae6
    asm call near ptr fcad0
    asm mov ax,g1e92
    asm mov g1e90,ax
    asm db 05dh
}
