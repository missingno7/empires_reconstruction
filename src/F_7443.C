struct R { char pad[32]; unsigned offsets[1]; };
extern struct R far *gc0ee;
extern int f1ea5();
extern void f03c9();
void f7443(void) { register int i; i=f1ea5(); f03c9(244+i*16,186,(char far *)gc0ee + gc0ee->offsets[i] + 2); }
