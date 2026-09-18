extern unsigned far *g0dc8;
extern int face7();
extern void f7856();
void f9402(int i) { register int flag; flag=i && !face7(); f7856((char far *)g0dc8 + g0dc8[i] + 2,flag); }
