/* F_CA91 -- preserve the caller's live AL and widen it into DGROUP. */
extern unsigned stream_note_delay;
void stream_note_delay_set()
{
    asm push bp
    asm mov bp,sp
    asm xor ah,ah
    asm mov stream_note_delay,ax
    asm pop bp
}
