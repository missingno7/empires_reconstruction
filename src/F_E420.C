/* F_E420 -- write OPL register 0xBD, ORing the two flags at DS:C6AA and
   DS:C6B5 into it.  Rule 9 both ways: the plain `= cond ? K : 0` writes the
   register variable straight, and the `|= cond ? K : 0` routes through AX
   first. */
extern void fc898();
extern char gc6aa;                      /* DS:C6AA */
extern char gc6b5;                      /* DS:C6B5 */

void fe420()
{
    register int v;

    v = gc6aa ? 0x80 : 0;
    v |= gc6b5 ? 0x40 : 0;
    fc898(0xbd, v);
}
