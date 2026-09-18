extern int f020f(), f6cea(), f01ce(), f0355(), f03a8(), f6ca6(), f6d3c(), f039f();
extern int gb83, gc0fc;
f778b(p,a,b,c,e)
char *p;
int a,b,c,e;
{
 char cap[15] = "\027\030 to Continue";
 register int s,d;
 s=f020f(); d=f6cea();
 gb83=4; gc0fc=b;
 f01ce(0); f0355(6,0xa2,0x134,0x24);
 f01ce(b); f03a8(8,0xa3,0x130,0x22);
 f01ce(a); f6ca6(c); f6d3c(12,0xa5,p);
 if(e) f6d3c(0xaa,0xb9,&cap);
 f039f(6,0xa2,0x134,0x24); f01ce(s); f6ca6(d);
}
