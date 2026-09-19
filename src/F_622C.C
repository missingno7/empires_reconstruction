extern int gb3e,gb40;
extern unsigned char gc0c8;
extern void hardretn(),hardresume();
void f622c(a,b)
int a,b;
{
    gb3e=1;
    gc0c8=b&0xff;
    if(gb40)hardretn(3);else hardresume(2);
    return 2;
}
