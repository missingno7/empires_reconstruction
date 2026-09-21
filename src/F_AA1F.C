/* Exact Turbo C recovery. Local order and final switch-case fallthrough
 * reproduce the original frame and branch layout. */
extern int gc46e,gc57e;
extern char near g1356[],g12e5[],g13b1[];
extern char far *g13b8;
struct record { char bytes[27]; };
extern struct record gc470[];
extern int fa15e(),f6b66(),faf45(),fa24e();
extern int fa036(char far *,char far *,struct record far *,char far *,long);
extern int f86c9(char far *);
int faa1f(void)
{
 int old,result;
 char buffer[200];
 register int selected,count;
 if(gc46e==1) {selected=gc57e;count=selected+1;}else {selected=0;count=gc57e;}
 while(1) {
  fa15e(old=selected);f6b66();
  switch(faf45()) {
   case 0x150: ++selected;selected%=count;break;
   case 0x148: --selected;selected=(selected+count)%count;break;
   case 27:
    fa15e(old);if(gc46e==1)return -1;
    gc46e=1;return -2;
   case 13:
    fa15e(old);
    if(selected==gc57e) {gc46e=0;return -2;}
    else if(gc46e==2) {
     fa036(buffer,g12e5,gc470+selected,g1356,0L);
     g13b8=buffer;
     if((result=f86c9(g13b1))>=0) {gc46e=1;if(result)fa24e(selected);}
     return -2;
    }
    return selected;
   case 18:
    if(gc46e==1) {gc46e=2;fa15e(old);return -2;}
  }
  fa15e(old);
 }
}
