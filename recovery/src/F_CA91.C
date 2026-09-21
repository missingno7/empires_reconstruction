/* F_CA91 -- preserve the caller's live AL and widen it into DGROUP. */
extern unsigned g1e92;
void fca91()
{
    asm push bp
    asm mov bp,sp
    asm xor ah,ah
    asm mov g1e92,ax
    asm pop bp
}
