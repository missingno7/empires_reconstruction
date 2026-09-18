extern char far *gbfbc, far *gc5ca, far *g40c4;
extern char gb6cf;
extern void f4b0c(), f1ecd(), f6c57(), f4e9f(), f6c6f();
void fb967(void) { *gbfbc=7; f4b0c(); f1ecd(); while(!gb6cf) { f6c57(24); g40c4=gc5ca; f4e9f(); f4b0c(); f1ecd(); f6c6f(); } }
