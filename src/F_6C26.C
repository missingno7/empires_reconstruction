/* F_6C26 -- spin until the 32-bit counter at DS:0B76 has advanced by n.
   The comparison is `jb` twice, so both sides are UNSIGNED long
   (tc20-codegen rule 5); the parameter is widened with `cwd`, so IT is a
   signed int.  The loop body is empty, which is why the jump to the test is
   the zero-displacement EB00. */
extern unsigned long gb76;              /* DS:0B76 */

void f6c26(n)
int n;
{
    unsigned long t;

    t = n + gb76;
    while (gb76 < t) ;
}
