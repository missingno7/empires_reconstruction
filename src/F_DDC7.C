/* F_DDC7 -- fill the 18 bytes at DS:CA50 with 0x7F.  The counter is the one
   register variable (SI), the index is signed (`jl`), and the pre-test loop
   opens with the jump to the test that TC 2.0 emits for `for`. */
extern unsigned char gca50[];           /* DS:CA50 */

void fddc7()
{
    register int i;

    for (i = 0; i < 18; i++)
        gca50[i] = 0x7f;
}
