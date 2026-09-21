/* F_C59A -- TC frame with a hand-written body, the F_CA83 shape: the frame is
   forced by the function HAVING A PARAMETER, which the asm body ignores and
   reads out of AL directly. */
extern int g1788;                       /* DS:1788 */

void sound_param_scale4(n)
unsigned char n;
{
    asm xor ah,ah
    asm shl ax,1
    asm shl ax,1
    asm mov word ptr g1788,ax
}
