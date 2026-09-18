extern int f020f(),f6cea(),f6ca6(),f7e07(),f8434(),f01ce(),f03a8(),f0355(),f03a5(),f03a2(),f6d3c(),f8267(),f8378(),f039f();
extern int gc11a,gc108,gc106,gc10a,gc122,gc128,gc12a,gc104,gc110,gc120,gc10c;
struct R{int a;char *b;char c;char *d;unsigned char e;};
f8480(p,n) struct R *p;int n;
{int a,b,x,w,h;register int y,i;
a=f020f();b=f6cea();f6ca6(0);f7e07(p);if(n)f8434();
f01ce(15);f03a8(gc10a,gc106,gc108,gc11a);
x=gc10a+4;y=gc106+2;if(p->a!=2)y+=2;w=gc108-10;h=gc11a-8;if(p->a!=2)h-=2;
f01ce(0);for(i=0;i<2;i++){w+=2;h+=2;f0355(--x,--y,w,h);}
f01ce(0);for(i=0;i<2;i++)f03a5(x+w+i,y+2,h);
for(i=0;i<2;i++)f03a2(x+2,y+h+i,w);
x+=2;y+=2;w-=4;
if(p->b){y+=2;f01ce(0);f6d3c(gc122,y,p->b);y+=11;f01ce(0);f03a2(x,y,w);}
y+=gc128;f01ce(0);f6d3c(x+gc12a,y,p->d);
if(gc104){f01ce(0);f03a2(x,gc110-2,w);f01ce(0);f6d3c(gc120,gc110,(char *)(char near *)gc104);}
if(p->a==4||p->a==5||p->a==6){f01ce(0);f03a2(x,gc10c-2,w);}
switch(p->a){case 5:case 6:case 7:f8267(1);case 4:f8267(0);f8378(p->e);break;}
f039f(gc10a,gc106,gc108,gc11a);f01ce(a);f6ca6(b);
}

