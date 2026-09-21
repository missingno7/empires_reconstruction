extern int f020f(), sprite_sheet_index_get();extern void f03a8();extern void f03a5();extern void f03a2();extern void f039f();
extern void sprite_sheet_select();
extern void rect_border_draw();
extern void gfx_color_select(int n);
extern void text_draw_wrapped(int,int,char far *);
struct R{char *a;int b;char pad[12];int c;};struct S{int n;struct R *p;};extern struct S *gc0fe;
void menu_list_draw(int n)
{int a,b,i,j;struct R r;int w;register int x,y;
a=f020f();b=sprite_sheet_index_get();sprite_sheet_select(0);gfx_color_select(0);f03a8(0,0,320,13);gfx_color_select(15);f03a8(1,2,318,10);
for(i=gc0fe->n-1;i>=0;i--){r=gc0fe->p[i];w=r.b;x=r.c;y=2;
if(i==n){gfx_color_select(12);f03a8(x,y,w+2,10);gfx_color_select(15);text_draw_wrapped(++x,y,r.a);}
else{for(j=0;j<2;j++){gfx_color_select(7);f03a5(x,y,9);f03a2(x,y+9,w+2);gfx_color_select(0);f03a2(x++,y--+9,1);}
rect_border_draw(x++,y,w+2,10);gfx_color_select(15);f03a8(x,y,w,9);gfx_color_select(0);text_draw_wrapped(x,y,r.a);}}
f039f(0,0,320,12);gfx_color_select(a);sprite_sheet_select(b);
}
