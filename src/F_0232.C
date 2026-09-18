extern void movmem(char far *s, char far *d, unsigned n);  /* CC.LIB MOVMEM, _TEXT+0xF348 */
extern char g9c[], g3904[], gde[], gbe[];  /* DGROUP+0x009C/0x3904/0x00DE/0x00BE */
void f0232(void)
{
    movmem(g9c, g3904, 0x20);
    movmem(gde, gbe, 0x20);
}
