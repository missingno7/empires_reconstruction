/* F_C988 -- note dispatch in the sound engine.  Both arguments arrive in
   AH/AL; TC has no register convention, so the body is asm and the two
   declared parameters exist only to force the frame. */
extern unsigned int notetab[];          /* DS:17FC */
/*@SYM _notetab=0x17FC kind=g key=storage_objects/G_P2132C.phys*/
extern void fcae6();
extern void fcad0();

void fc988(note, shift)
unsigned char note, shift;
{
    asm mov cl,ah
    asm mov bx,offset notetab
    asm dec al
    asm shl al,1
    asm xor ah,ah
    asm add bx,ax
    asm mov ax,[bx]
    asm shr ax,cl
    asm call near ptr fcae6
    asm call near ptr fcad0
}
