/* F_D818 -- reset the byte counter at DS:2F30. */
void fd818()
{
    asm db 055h,08bh,0ech,057h,0bfh,030h,02fh,0c6h,05h,0
    asm db 05fh,05dh
}
