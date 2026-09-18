/* F_6C87 -- has the 32-bit counter at DS:0B76 reached the deadline at
   DS:C0D0?  `ja`/`jb`/`jae` on the two halves makes both sides UNSIGNED long
   (rule 5), and one convention name reaches each high word as `_sym+2`
   (rule 18).  Both arms END in a jump to the epilogue -- the second one is
   the zero-displacement EB00 -- so both are `return` statements of an
   `if`; a single `return (a < b ? 0 : 1)` lets the second arm fall through
   and loses the EB00. */
extern unsigned long gb76;              /* DS:0B76, high word at DS:0B78 */
extern unsigned long gc0d0;             /* DS:C0D0, high word at DS:C0D2 */

int f6c87()
{
    if (gb76 < gc0d0)
        return (0);
    return (1);
}
