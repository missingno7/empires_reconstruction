extern char far *gbfc0;

char far *f2a2d(int n)
{
    unsigned char far *p;
    int i;

    p = (unsigned char far *) (gbfc0 + (*gbfc0) * 4 + 2);
    for (i = 0; i < n; i++)
        p += *p;
    return (char far *) p;
}
