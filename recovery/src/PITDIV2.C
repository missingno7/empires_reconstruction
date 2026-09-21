/* F_CAE6 -- program PIT channel 2 with the divisor already in AX.  The
   parameter is declared (and ignored by the body) purely to force the frame. */
void pit_channel2_set_divisor(div)
unsigned int div;
{
    asm out 42h,al
    asm mov al,ah
    asm out 42h,al
}
