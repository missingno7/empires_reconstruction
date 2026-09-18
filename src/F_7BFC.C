extern int f020f(),f6cea(),f6ca6(),f01ce(),f03a8(),f6d3c(),f03a5(),f03a2(),f0355(),f039f();
struct R{char *a;int b;char pad[12];int c;};struct S{int n;struct R *p;};extern struct S *gc0fe;
f7bfc(n) int n;
{int a,b,i,j;struct R r;int w;register int x,y;
a=f020f();b=f6cea();f6ca6(0);f01ce(0);f03a8(0,0,320,13);f01ce(15);f03a8(1,2,318,10);
for(i=gc0fe->n-1;i>=0;i--){r=gc0fe->p[i];w=r.b;x=r.c;y=2;
if(i==n){f01ce(12);f03a8(x,y,w+2,10);f01ce(15);f6d3c(++x,y,r.a);}
else{for(j=0;j<2;j++){f01ce(7);f03a5(x,y,9);f03a2(x,y+9,w+2);f01ce(0);f03a2(x++,y--+9,1);}
f0355(x++,y,w+2,10);f01ce(15);f03a8(x,y,w,9);f01ce(0);f6d3c(x,y,r.a);}}
f039f(0,0,320,12);f01ce(a);f6ca6(b);
}
