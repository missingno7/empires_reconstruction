/* F_A15E -- draw one menu row's background twice, through the two
   runtime-generated thunks at IP 03ABh and 039Fh.  `mov sp,bp` after each
   call rather than `add sp,8` is rule 13: no register variable is saved, so
   SP can simply be restored. */
extern void f03ab();
extern void f039f();

void fa15e(n)
int n;
{
    f03ab(0x28, n * 11 + 0x3c, 0xf5, 10);
    f039f(0x28, n * 11 + 0x3c, 0xf5, 10);
}
