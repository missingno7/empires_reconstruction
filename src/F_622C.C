extern int gb3e,gb40;
extern unsigned char gc0c8;
extern void fF7CD(),fF7B2();
void f622c(a,b)
int a,b;
{
    gb3e=1;
    gc0c8=b&0xff;
    if(gb40)fF7CD(3);else fF7B2(2);
    return 2;
}
