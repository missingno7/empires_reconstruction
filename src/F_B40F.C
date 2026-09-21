/* Exact C recovery of compact descriptor expansion; far offsets are bytes. */
extern char near g9bfc[],gbf66[];
extern char far *gc5c6,*gc592,*gc596,*gb07c[];
extern int f656c();
extern int f684a(int,char far * far *);
extern int memmove(void far *,void far *,unsigned);
void fbb40f(void)
{
 register int i,j;
 i=0;
 f656c(0x57);memmove(g9bfc,gc5c6,0x54);
 f656c(0x58);memmove(gbf66,gc5c6,0x54);
 f684a(0x55,&gc592);
 for(j=0;j<2;j++) gb07c[i++]=gc592+((int far *)gc592)[j]+2;
 f684a(0x56,&gc596);
 for(j=0;j<12;j++) gb07c[i++]=gc596+((int far *)gc596)[j]+2;
 while(i<84) gb07c[i++]=gb07c[0];
}
