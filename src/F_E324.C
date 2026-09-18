/* F_E324 -- program the voice's carrier level/multiplier pair (OPL base 0x80)
   from members 4 and 7 of the 14-byte record.  Twin of F_E2D6, which does the
   same with members 3 and 6 and OPL base 0x60. */
extern void fc898();
extern char g2fe4[];                    /* DS:2FE4, byte per voice */
struct r14 { char c0, c1, c2, c3, c4, c5, c6;
             char c7, c8, c9, c10, c11, c12, c13; };
extern struct r14 gc91b[];              /* DS:C91B, 14 bytes per voice */

void fe324(v)
int v;
{
    register int d;

    d = gc91b[v].c4 << 4;
    d |= gc91b[v].c7 & 0xf;
    fc898(g2fe4[v] + 0x80, d);
}
