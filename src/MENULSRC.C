#include "GC0FE.H"
extern char g0bb4[];
extern void menu_list_draw(), menu_list_enable(), menu_list_disable(void);
void menu_list_source_set(char far *p) { if(p) { gc0fe=p; menu_list_draw(-1); menu_list_enable(); } else { gc0fe=g0bb4; menu_list_disable(); } }
