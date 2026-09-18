extern int gb7e;
extern void f7298(void);

int f727d(void)
{
    if (++gb7e >= 3)
        gb7e = 0;
    f7298();
    return gb7e;
}
