/* F_6C6F -- spin until the deadline armed by f6c57 has passed.  The EB 00 at
   the entry is the while-loop's jump-to-test with an empty body. */
extern unsigned long gb76, gc0d0;

void f6c6f(void)
{
    while (gb76 < gc0d0)
        ;
}
