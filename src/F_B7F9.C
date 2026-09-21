/* Exact Turbo C reconstruction. Board fields use the existing workspace
 * arrays; no new storage or interior-address globals are introduced. */
extern int f656c(),fcb48(),fcaf1(),f6c57(),f03b4(),f4b0c(),f2269(),f039f(),f6c6f(),fd5f9(),f4e9f(),f1ecd();
extern int f03cc(int,int,void far *,int);
extern int gbc,g1774,g96;
extern int near gb52f[];
extern unsigned char gb6cf;
extern unsigned char near gb3af[];
extern char far *gc5c6,*gc5ca,*g40c4,*gbfbc;
void fb7f9(void)
{
 register int x,i;
 f656c(0x48);gbc=0;gb3af[136]=1;gb3af[200]=1;gb3af[264]=1;gb3af[328]=1;
 fcb48();g1774=1;fcaf1(25);
 for(x=118;x<=198;x+=4) {
  f6c57(24);f03b4(x-4,355,92,117,x-4,27);f03cc(x,27,gc5c6,0);
  gb52f[33]+=4;gb52f[49]+=4;gb52f[65]+=4;gb52f[97]+=4;gb52f[129]+=4;gb52f[161]+=4;
  f4b0c();f2269();f039f(x-4,27,96,117);f6c6f();
 }
 g1774=0;fcb48();f2269();g96=488;f03cc(x-4,355,gc5c6,0);
 f03b4(6,344,308,144,6,200);g96=159;gbc=1;gb6cf=0;fd5f9(69);
 for(i=0;i<20;i++) {
  f6c57(24);g40c4=gc5ca;f4e9f();f4b0c();f2269();f1ecd();f6c6f();
 }
 for(i=433;i<=440;i++) gbfbc[i]=7;
}
