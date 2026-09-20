/* Exact Turbo C recovery of the 385-byte selection/workspace routine.
   The far-pointer fa28d prototype and local declaration order are byte-significant. */
extern int f6b74(),f7925(),fcb48(),fc834(),fd593(),fa09d(),f03c9(),f01ce(),fa223(),f9f40(),f6990(),f6b66(),fa33f(),faa1f(),fa85e(),f86c9(),fa13f(),f6997(),f03a8(),f039f(),fd5a6();
extern int g237e,g98,g9a,gc57e,gc46e,g176e,g1772,g13ed;
extern char near g139d[];
extern char far *gc5c6;
struct record { char first[13]; int descriptor1,descriptor2; char rest[10]; };
extern struct record gc470[];
extern int fa28d(struct record far *, int, int);
int fab66(void)
{
 int y,saved;
 register int selected,i;
 saved=f6b74(); f7925(); fcb48(); fc834();
 g237e=-1; fd593(); g98=0; g9a=159; fa09d();
 f03c9(0,200,gc5c6); f01ce(0); gc57e=fa223();
 y=249;
 for(i=0;i<gc57e;i++) fa28d(gc470+i,y+=11,1);
 f9f40(0,200,320,200,0,0);
 f03c9(0,200,gc5c6); gc46e=1; f6990();
 do {
   if(!(gc57e=fa223()))gc46e=0;
   f6b66(); fa33f();
   if(gc46e) selected=faa1f(); else selected=fa85e();
   if(selected==-1 && f86c9(g139d)!=1)selected=-2;
 } while(selected < -1);
 fa13f(); if(!saved)f6997(); f01ce(1);
 f03a8(0,0,320,200); f039f(0,0,320,200);
 g98=4;g9a=155;
 g176e=((struct record far *)&gc470[selected])->descriptor1;
 g1772=((struct record far *)&gc470[selected])->descriptor2;
 fd5a6(); return g13ed=selected;
}

