/* F_E27B -- push one voice's pitch/flag word to the OPL through fc898.
   The 14-byte record array at DS:C91B is indexed with a `mul`, which is how
   TC 2.0 reaches a non-power-of-two stride; the two member reads recompute
   the address independently (TC 2.0 does no CSE, tc20-codegen rule 6). */
extern void fc898();
extern char g2ff6[];                    /* DS:2FF6, byte per voice */
extern char g3008[];                    /* DS:3008, byte per voice */
struct r27b { char c0, c1, c2, c3, c4, c5, c6;
              char c7, c8, c9, c10, c11, c12, c13; };
extern struct r27b gc91b[];             /* DS:C91B, 14 bytes per voice */

void fe27b(v)
register int v;
{
    register int d;

    if (g2ff6[v]) return;
    d = gc91b[v].c2 * 2;
    d |= gc91b[v].c12 ? 0 : 1;
    fc898(g3008[v] + 0xc0, d);
}
