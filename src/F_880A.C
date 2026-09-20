/* Exact Turbo C reconstruction of the selection loop and its shared-frame
 * cleanup (formerly F_8C04). Packed layouts and local declaration order
 * reproduce the original 30-byte frame. No synthetic cleanup entry. */
struct input { char far *title; char flag; char far * far *records; signed char count; int a,b,c,d; };
struct view { int mode; char far *title; char flag; char far *text; unsigned char direction; int a,b,c,d; };
extern int f6b74(),f6990(),f6b66(),f703e(),fd593(),f792c(),f7925(),f6b1a(),f8378(),f8414(),f8453(),f791e(),fd5a6(),f7162(),f6997();
extern int f8480(struct view far *,int);
int f880a(struct input far *p)
{
 struct view v;
 int outer,state,first,done,key;
 register int selected,direction;
 selected=0; first=1; direction=0;
 outer=f6b74(); f6990(); f6b66(); f703e(); fd593();
 state=f792c(); f7925();
 v.title=p->title; v.flag=p->flag;
 v.a=p->a;v.b=p->b;v.c=p->c;v.d=p->d;
 while(selected>=0 && selected<p->count) {
  if(selected==0) { v.mode=4; direction=v.direction=0; }
  else { if(selected<p->count-1) v.mode=5; else v.mode=6; direction=v.direction=1; }
  v.text=p->records[selected];
  f8480(&v,first); first=0;done=0;
  while(!done) {
   key=f6b1a();
   switch(key) {
    case 9: case 0x148: case 0x14b: case 0x14d: case 0x150:
     if(selected>0) { f8378(direction);direction=!direction;f8378(direction); } break;
    case 13: f8414(direction);done=1;break;
    case 27: direction=-1;done=1;break;
    case 0x13b: direction=2;done=1;break;
    case 0x149:
     if(selected>0) { if(direction>0) f8378(direction);direction=0;f8414(0);done=1; } break;
    case 0x151:
     if(selected==0) { direction=0;f8414(0);done=1; }
     else if(selected<p->count-1) {direction=1;f8414(1);done=1;} break;
   }
  }
  switch(direction) {
   case -1:selected=-1;break;
   case 0:if(selected==0) ++selected;else --selected;break;
   case 1:++selected;break;
   case 2:selected=p->count;break;
  }
 }
 f8453();if(state) f791e();fd5a6();f7162();if(!outer) f6997();f6b66();
 if(selected<0) return 0; else return 1;
}
