/* Exact Turbo C reconstruction of the menu loop and shared-frame cleanup.
 * The former F_7DD3 early-return target is this function's epilogue.
 * Preserve local declaration and comparison evaluation order. */
struct record { char head[6]; int count; char middle[4]; int (** far callbacks)(); int width,x; };
struct catalog { int count; struct record far *records; };
extern struct catalog far *gc0fe;
extern int f792c(),f7925(),f6b74(),f6990(),f6b66(),f703e(),fd593(),f7bfc(),f03ab(),f039f(),f6b1a(),f8453(),f791e(),fd5a6(),f7162(),f6997();
extern int f7d91(struct record far *);
void f7964(int initial)
{
 int outer,state,oldselected;
 struct record far *p;
 int oldrow,done,key,result,cleared;
 register int selected,row;
 selected=initial;
 if(selected<0 || selected>=gc0fe->count) return;
 state=f792c();f7925();outer=f6b74();f6990();f6b66();f703e();fd593();
 while(selected!=-1) {
  oldselected=selected;p=&gc0fe->records[selected];f7bfc(selected);f7d91(p);
  cleared=0;row=0;done=0;
  while(!done) {
   oldrow=row;
   f03ab(p->x+4,row*10+17,p->width,10);f039f(p->x+4,row*10+17,p->width,10);
   key=f6b1a();
   switch(key) {
    case 0x148: if(--row<0) row=p->count-1;break;
    case 0x150: if(++row>=p->count) row=0;break;
    case 0x14b: if(--selected<0) selected=gc0fe->count-1;done=1;break;
    case 0x14d: if(gc0fe->count<=++selected) selected=0;done=1;break;
    case 0x13b:case 0x13c:case 0x13d:case 0x13e:case 0x13f:case 0x140:case 0x141:case 0x142:case 0x143:case 0x144:
     selected=key-0x13b;
     if(selected<0 || selected>=gc0fe->count) selected=oldselected;
     else if(selected!=oldselected) done=1;
     break;
    case 27:done=1;selected=-1;break;
    case 13:
     f8453();cleared=1;f7bfc(-1);
     if(p->callbacks[row]) result=p->callbacks[row]();else result=1;
     switch(result) {
      case 0:default:done=1;selected=-1;break;
      case 1:done=1;break;
     }
     break;
   }
   if(!done) {
    f03ab(p->x+4,oldrow*10+17,p->width,10);f039f(p->x+4,oldrow*10+17,p->width,10);
   }
  }
  if(!cleared) f8453();
 }
 f7bfc(-1);if(state) f791e();fd5a6();f7162();if(!outer) f6997();f6b66();
}

