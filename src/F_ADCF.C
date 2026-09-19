#ifndef EMPIRES_RECORD27_DECLARED
#define EMPIRES_RECORD27_DECLARED
struct R{char a[9];int w9;char b11,b12;char pad13[4];int w17;char pad19[2];char b21,b22,b23,b24,b25,b26;};
#endif
extern struct R gc360[],gc470[];extern int g13ed;extern int fa24e(),fa13f();fadcf(){register int i;for(i=0;i<10;i++){if(!gc360[i].a[0]){gc360[i]=gc470[g13ed];break;}}if(i==10){for(i=1;i<10;i++)gc360[i-1]=gc360[i];gc360[9]=gc470[g13ed];}gc470[g13ed].b11=32;gc470[g13ed].b21=4;gc470[g13ed].w9=1;gc470[g13ed].w17=0;gc470[g13ed].b12=0;gc470[g13ed].b22=0;gc470[g13ed].b23=0;gc470[g13ed].b24=0;gc470[g13ed].b25=0;gc470[g13ed].b26=0;fa13f();}