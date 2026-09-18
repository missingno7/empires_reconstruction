/* F_E44B -- the last of the per-voice OPL writes (base 0xE0): the record's
   member 13, masked, but only while the enable word at DS:CA22 is set. */
extern void fc898();
extern int gca22;                       /* DS:CA22 */
extern char g2fe4[];                    /* DS:2FE4, byte per voice */
struct r14 { char c0, c1, c2, c3, c4, c5, c6;
             char c7, c8, c9, c10, c11, c12, c13; };
extern struct r14 gc91b[];              /* DS:C91B, 14 bytes per voice */

void fe44b(v)
int v;
{
    register int d;

    if (gca22) d = gc91b[v].c13 & 3;
    else d = 0;
    fc898(g2fe4[v] + 0xe0, d);
}
