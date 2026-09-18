struct R { char pad[11]; char b; char rest[15]; };
extern struct R gc470[];
extern int g13ed, gc5b0, gc5b2[];
void fce68(int i) { if(i!=-1) { gc5b0++; gc470[g13ed].b |= (gc5b2[i]=1<<i); } }
