/* F_DA20 -- clamp the tempo to 1..12, latch it, and recompute the derived
   word.  `jbe`/`jae` make the clamp UNSIGNED and the `mul` is an unsigned
   int multiply (tc20-codegen rule 5); the second statement re-reads the
   global rather than reusing the register, TC 2.0 doing no CSE (rule 6). */
extern int gc6c1;                       /* DS:C6C1 */
extern int gca6b;                       /* DS:CA6B */

void fda20(v)
register unsigned v;
{
    if (v > 12) v = 12;
    if (v < 1) v = 1;
    gc6c1 = v;
    gca6b = gc6c1 * 0x19;
}
