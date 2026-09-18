/* F_E262 -- write OPL register 0x08 with bit 6 set from the flag at DS:C6B4.
   `v = cond ? K : 0` writes the destination register straight (rule 9); here
   the destination is the argument slot. */
extern void fc898();
extern char gc6b4;                      /* DS:C6B4 */

void fe262()
{
    fc898(8, gc6b4 ? 0x40 : 0);
}
