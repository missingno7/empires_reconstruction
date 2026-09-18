/* F_E54D -- is an OPL chip there?  Reset both timers, read the status, arm
   timer 1, spin 200 port reads to let it expire, read the status again and
   reset.  The two register variables are declared counter-first (rule 12:
   SI then DI in DECLARATION order, not in order of first use).  The verdict
   is a plain `&&`, which is why both arms end at one EB00. */
extern void fc898();
extern int inport();
extern unsigned g1830;                  /* DS:1830 */

int fe54d()
{
    unsigned t;
    register unsigned i;
    register unsigned d;

    fc898(4, 0x60);
    fc898(4, 0x80);
    d = inport(g1830);
    fc898(2, 0xff);
    fc898(4, 0x21);
    for (i = 0; i < 200; i++)
        inport(g1830);
    t = inport(g1830);
    fc898(4, 0x60);
    fc898(4, 0x80);
    return (!(d & 0xe0) && (t & 0xe0) == 0xc0);
}
