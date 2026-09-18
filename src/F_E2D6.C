/* F_E2D6 -- twin of F_E324: members 3 and 6, OPL base 0x60. */
extern void fc898();
extern char g2fe4[];                    /* DS:2FE4, byte per voice */
struct r14 { char c0, c1, c2, c3, c4, c5, c6;
             char c7, c8, c9, c10, c11, c12, c13; };
extern struct r14 gc91b[];              /* DS:C91B, 14 bytes per voice */

void fe2d6(v)
int v;
{
    register int d;

    d = gc91b[v].c3 << 4;
    d |= gc91b[v].c6 & 0xf;
    fc898(g2fe4[v] + 0x60, d);
}
