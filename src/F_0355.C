extern void f03a2(int x, int y, int n);
extern void f03a5(int x, int y, int n);

void f0355(int x, int y, int w, int h)
{
    f03a2(x, y, w);
    f03a5(x, y, h);
    f03a2(x, y + h - 1, w);
    f03a5(x + w - 1, y, h);
}
