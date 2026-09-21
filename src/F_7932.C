#include "GC0FE.H"
extern char g0bb4[];
extern void menu_list_draw(), f791e(), f7925(void);
void f7932(char far *p) { if(p) { gc0fe=p; menu_list_draw(-1); f791e(); } else { gc0fe=g0bb4; f7925(); } }
