extern int f86c9();
extern void fa13f();
extern void longjmp();
extern char g21b0[], g8bfe[];
void fce00(void)
{
    register int si;
    si = f86c9(g21b0);
    if (si == 1) {
        fa13f();
        longjmp(g8bfe, 1);
    }
    return 0;
}
