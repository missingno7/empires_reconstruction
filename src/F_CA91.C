/* F_CA91 -- preserve the caller's live AL and widen it into DGROUP. */
extern unsigned g1e92;
void fca91()
{
    asm db 055h,08bh,0ech
    asm xor ah,ah
    asm mov g1e92,ax
    asm db 05dh
}
