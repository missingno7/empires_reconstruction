extern int f86c9();
extern void longjmp();
extern char g219c[], g8bfe[];
void fcddd(void)
{
    if (f86c9(g219c) == 1)
        longjmp(g8bfe, 2);
    return 0;
}
