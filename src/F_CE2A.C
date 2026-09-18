extern int f86c9();
extern void fd593(), fa13f(), fcb48(), fc834(), fd5a6();
extern void longjmp();
extern char g21ec[], g8bfe[];
extern int g1772, g176e;
void fce2a(void)
{
    register int si;
    fd593();
    si = f86c9(g21ec);
    if (si == 1) {
        fa13f();
        g176e = g1772 = 0;
        fcb48();
        fc834();
        longjmp(g8bfe, 3);
    }
    fd5a6();
    return 0;
}

