/* F_D386 main -- decode one far resource-table record.  The JS branch targets
   the separately retained 11-byte tail immediately after this C extent. */
extern char far *gbfc4;
extern char far *a72b2[];
extern void f03c9();
void fd386(dummy)
int dummy;
{
    asm db 057h,056h,0fch
    asm db 0c5h,036h
    asm dw gbfc4
    asm db 0ach,033h,0c9h,08ah,0c8h,051h,033h,0dbh,08bh,0feh
    asm db 0ach,08ah,0d8h,0adh,08bh,0c8h,003h,0f3h,0ach,0feh,0c8h
    asm db 078h,028h,0feh,005h,08ah,0d8h,0d1h,0e3h,0d1h,0e3h
    asm db 0c4h,0b7h
    asm dw a72b2
    asm db 006h,056h,033h,0d2h,086h,0d5h,052h,0d1h,0e1h,051h
    asm call near ptr f03c9
    asm db 083h,0c4h,8,08bh,0f7h,083h,0c6h,0ch,059h,0e2h,0cah,05eh,05fh
}
