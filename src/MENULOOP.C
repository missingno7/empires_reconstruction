/* Exact Turbo C reconstruction of the menu loop and shared-frame cleanup.
 * The former F_7DD3 early-return target is this function's epilogue.
 * Preserve local declaration and comparison evaluation order. */
#include "GC0FE.H"
#define g0fecat ((struct gc0fe_catalog far *) gc0fe)
extern int menu_list_active(), keyboard_chain_active(), f6b1a();extern void gfx_fill_rect();extern void gfx_box();
extern void menu_list_disable(void);
extern void keyboard_chain_enable(void);
extern void keyboard_buffer_drain(void);
extern void sound_start(void);
extern void ui_overlay_hide(void);
extern void ui_overlay_show(void);
extern void menu_list_enable(void);
extern void menu_list_draw();
extern void dialog_restore_screen();
extern void sound_request_count_dec(void);
extern void keyboard_chain_disable(void);
extern int dialog_draw_panel(struct gc0fe_record far *);
void menu_loop_run(int initial)
{
 int outer,state,oldselected;
 struct gc0fe_record far *p;
 int oldrow,done,key,result,cleared;
 register int selected,row;
 selected=initial;
 if(selected<0 || selected>=g0fecat->count) return;
 state=menu_list_active();menu_list_disable();outer=keyboard_chain_active();keyboard_chain_enable();keyboard_buffer_drain();ui_overlay_show();sound_start();
 while(selected!=-1) {
  oldselected=selected;p=&g0fecat->records[selected];menu_list_draw(selected);dialog_draw_panel(p);
  cleared=0;row=0;done=0;
  while(!done) {
   oldrow=row;
   gfx_fill_rect(p->x+4,row*10+17,p->width,10);gfx_box(p->x+4,row*10+17,p->width,10);
   key=f6b1a();
   switch(key) {
    case 0x148: if(--row<0) row=p->count-1;break;
    case 0x150: if(++row>=p->count) row=0;break;
    case 0x14b: if(--selected<0) selected=g0fecat->count-1;done=1;break;
    case 0x14d: if(g0fecat->count<=++selected) selected=0;done=1;break;
    case 0x13b:case 0x13c:case 0x13d:case 0x13e:case 0x13f:case 0x140:case 0x141:case 0x142:case 0x143:case 0x144:
     selected=key-0x13b;
     if(selected<0 || selected>=g0fecat->count) selected=oldselected;
     else if(selected!=oldselected) done=1;
     break;
    case 27:done=1;selected=-1;break;
    case 13:
     dialog_restore_screen();cleared=1;menu_list_draw(-1);
     if(p->callbacks[row]) result=p->callbacks[row]();else result=1;
     switch(result) {
      case 0:default:done=1;selected=-1;break;
      case 1:done=1;break;
     }
     break;
   }
   if(!done) {
    gfx_fill_rect(p->x+4,oldrow*10+17,p->width,10);gfx_box(p->x+4,oldrow*10+17,p->width,10);
   }
  }
  if(!cleared) dialog_restore_screen();
 }
 menu_list_draw(-1);if(state) menu_list_enable();sound_request_count_dec();ui_overlay_hide();if(!outer) keyboard_chain_disable();keyboard_buffer_drain();
}

