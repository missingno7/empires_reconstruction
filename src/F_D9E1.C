/* F_D9E1 -- turn the whole OPL bank on or off: latch the enable word, silence
   all 18 voices' 0xE0 registers, then write the enable word to register 1. */
extern void fc898();
extern int gca22;                       /* DS:CA22 */
extern char g2fe4[];                    /* DS:2FE4, byte per voice */

void fd9e1(f)
int f;
{
    register int i;

    gca22 = f ? 0x20 : 0;
    for (i = 0; i < 0x12; i++) fc898(g2fe4[i] + 0xe0, 0);
    fc898(1, gca22);
}
