struct R { char pad[13]; int a, b; char pad2[10]; };   /* sizeof == 27 == 0x1b */
extern struct R tbl[];
extern int n_sel, f1, f2;
extern void reset1(void), reset2(void), refresh(void);
void f68cf(void)
{
    if (f2) {
        reset1(); reset2();
        f2 = f1 = 0;
        if (n_sel >= 0) tbl[n_sel].b = tbl[n_sel].a = 0;
    } else {
        f2 = f1 = 1;
        if (n_sel >= 0) tbl[n_sel].b = tbl[n_sel].a = 1;
        refresh();
    }
}
