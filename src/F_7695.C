extern int f020f(), f6cea(), f01ce(), f03a8(), f6ca6(), f6d3c(), f039f();
extern int gb83, gc0fc;
extern char *gc0f6;
f7695(p)
char *p;
{
 char cap[13] = "\027\030 to Select";
 register int s,d;
 s=f020f(); d=f6cea();
 gb83=3; gc0f6=p; gc0fc=0;
 f01ce(0); f03a8(0,0xbc,0x140,12);
 f01ce(15); f6ca6(0);
 f6d3c(10,0xbd,p); f6d3c(0xaa,0xbd,&cap);
 f039f(0,0xbc,0x140,12); f01ce(s); f6ca6(d);
}
