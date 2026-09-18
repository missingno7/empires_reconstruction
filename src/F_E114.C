extern void fe0c0();
void fe114(v, q, w)
int v;
char far *q;
int w;
{
    int buf[14];
    register int i;
    for (i = 0; i < 13; i++)
        buf[i] = *q++;
    fe0c0(v, (char far *)buf, w);
}
