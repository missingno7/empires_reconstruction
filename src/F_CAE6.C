/* F_CAE6 -- program PIT channel 2 with the divisor already in AX.  The
   parameter is declared (and ignored by the body) purely to force the frame. */
void fcae6(div)
unsigned int div;
{
    asm out 42h,al
    asm mov al,ah
    asm out 42h,al
}
