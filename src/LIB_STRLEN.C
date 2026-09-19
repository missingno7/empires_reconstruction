/* Exact Turbo C reconstruction of the compact-model STRLEN contribution.
   The inline bytes preserve the historical far-pointer scan and return ABI. */
int strlen()
{
    asm db 055h,08bh,0ech,056h,057h,0fch,0c4h,07eh,04h
    asm db 032h,0c0h,0b9h,0ffh,0ffh,0f2h,0aeh,08bh,0c1h
    asm db 0f7h,0d0h,048h,0ebh,00h,05fh,05eh,05dh
}
