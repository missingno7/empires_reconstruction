extern char fad0e();
extern int gc5b0, gc5b2[];
void fd089(void) { register int flags,i; flags=fad0e(); gc5b2[0]=flags&1; gc5b2[1]=flags&2; gc5b2[2]=flags&4; gc5b2[3]=flags&8; i=0; gc5b0=i; for(;i<4;i++) if(gc5b2[i]) gc5b0++; }
