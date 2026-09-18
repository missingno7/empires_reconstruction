/* F_8414 -- draw the parameter four times, one tick apart.  A plain
   parameter with a `register` local gives the LOCAL si (rule 12), and there
   is no `sub sp` because the only local IS the register variable. */
extern void f8378();
extern void f6c26();

void f8414(a)
int a;
{
    register int i;

    for (i = 0; i < 4; i++) {
        f8378(a);
        f6c26(14);
    }
}
