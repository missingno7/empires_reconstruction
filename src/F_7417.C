struct R { char pad[22]; unsigned offsets[1]; };
extern struct R far *gc0ee;
extern int f1eb4();
extern void f03c9();
void f7417(void) { f03c9(244,175,(char far *)gc0ee + gc0ee->offsets[f1eb4()] + 2); }
