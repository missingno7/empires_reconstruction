/* F_CADB -- speaker gate off.  asm body, frame from the module's -k. */
/*@FLAGS -k*/
void speaker_gate_off()
{
    asm in  al,61h
    asm and al,0fch
    asm out 61h,al
}
