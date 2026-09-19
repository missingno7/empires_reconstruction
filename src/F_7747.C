extern int f792c(), f6b1a(), f778b();
extern void f7925(), f791e(), f6fda();
void f7747(char far *p) { register int key, saved; saved=f792c(); f7925(); f778b(p,0,15,1,0); do { key=f6b1a(); } while (key!=13 && key!=27); if(saved) f791e(); f6fda(); }
