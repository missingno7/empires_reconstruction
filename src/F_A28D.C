#include "C470.H"
extern int ultoa(),text_line_width();
extern void text_draw_wrapped(int,int,char far *);
extern void sprite_pool_draw_masked(char near *,int,unsigned);static char g1650[]="Explorer";static char g1659[]="Expert";int fa28d(p,y,a) struct c470_record far *p;register int y;int a;{register int w;char buf[6];char *s;if(p->text[0]){text_draw_wrapped(43,y,p);s=p->flags&16?(char *)g1650:(char *)g1659;text_draw_wrapped(0x92,y,s);if(!a){ultoa((long)p->value,buf,10);w=text_line_width(buf);text_draw_wrapped(0x102-w,y,buf);}else sprite_pool_draw_masked(0xdd,y,p->flags);}}
