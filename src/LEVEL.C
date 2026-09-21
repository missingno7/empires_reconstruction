/* src/LEVEL.C: Level play: display init, movement loop and chapter driver.
   One translation unit; the sections below were the separate member
   sources of grouped module C_AF45_C15E and keep their original ids. */

#include "LAYOUT.H"
#include "GB3AF.H"
#include "DIALOG.H"
#include "R3E8.H"

extern int resource_load_record();
extern void resource_load_record_alloc(int,char far * far *);
extern void far *memmove(void far *,void far *,unsigned);
extern unsigned char near g9bfc[],gbf66[];
extern void farfree();
extern int g720;
extern void timer_deadline_arm();

/* ---- F_AF45 (original code at 0xAF45) ---- */
/* Preincrement must feed the comparison: separate statements emit a shorter
   memory comparison. Explicit return preserves the final epilogue jump. */
extern void f03b4();
extern void f03ba();
extern void f039f();
extern int timer_deadline_reached();
extern int f6b4a();
extern int f6b1a();
extern int g13ef[];                     /* DS:13EF, the frame index */

int faf45()
{
    f03b4(g13ef[0] * 0x12, 0x190, 0x12, 0x21, 0x04, 0x55);
    f03ba(0x04, 0x55, 0x12, 0x21, 0x12a, 0x55);
    f03b4(0x04, 0x55, 0x12, 0x21, 0x04, 0x11d);
    f03b4(0x12a, 0x55, 0x12, 0x21, 0x12a, 0x11d);
    f039f(0x04, 0x55, 0x12, 0x21);
    f039f(0x12a, 0x55, 0x12, 0x21);
    timer_deadline_arm(0x17);
    while (!f6b4a()) {
        if (timer_deadline_reached()) {
            if (++g13ef[0] >= 3)
                g13ef[0] = 0;
            f03b4(g13ef[0] * 0x12, 0x190, 0x12, 0x21, 0x04, 0x55);
            f03ba(0x04, 0x55, 0x12, 0x21, 0x12a, 0x55);
            f039f(0x04, 0x55, 0x12, 0x21);
            f039f(0x12a, 0x55, 0x12, 0x21);
            timer_deadline_arm(0x17);
        }
    }
    f6b1a();
    return;
}


/* ---- F_B09A (original code at 0xB09A) ---- */
extern void f03cc();extern void f03c9();extern void f03b4();extern char far *ui_gfx_shadow_a;extern int g96;fb09a(){resource_load_record(0x47);f03c9(6,200,ui_gfx_shadow_a);f03b4(6,200,0x134,0x90,6,0x158);resource_load_record(0x48);g96=400;f03cc(0x72,0xd3,ui_gfx_shadow_a,0);g96=0x9f;f03b4(6,200,0x134,0x90,6,16);}


/* ---- F_B122 (original code at 0xB122) ---- */
/* Expand the board descriptor resources into byte-addressed far pointers. */
extern char far *ui_gfx_shadow_a,*gc5a8,*gc580,*gc5ac,*gc59a,*gc5a4;
extern char far *gc58a,*gc58e,*gc584,*gc59e,*resource_ptr_table[];
void board_resource_expand(void)
{
 register int i,j;
 i=0;
 resource_load_record(0x52);memmove(g9bfc,ui_gfx_shadow_a,0x54);
 resource_load_record(0x53);memmove(gbf66,ui_gfx_shadow_a,0x54);
 resource_load_record_alloc(0x49,&gc5a8);
 for(j=0;j<12;j++) resource_ptr_table[i++]=gc5a8+((int far *)gc5a8)[j]+2;
 resource_load_record_alloc(0x4a,&gc580);
 for(j=0;j<8;j++) resource_ptr_table[i++]=gc580+((int far *)gc580)[j]+2;
 resource_load_record_alloc(0x4b,&gc5ac);
 for(j=0;j<10;j++) resource_ptr_table[i++]=gc5ac+((int far *)gc5ac)[j]+2;
 resource_load_record_alloc(0x4c,&gc59a);
 for(j=0;j<17;j++) resource_ptr_table[i++]=gc59a+((int far *)gc59a)[j]+2;
 resource_load_record_alloc(0x4d,&gc5a4);
 for(j=0;j<4;j++) resource_ptr_table[i++]=gc5a4+((int far *)gc5a4)[j]+2;
 resource_load_record_alloc(0x4e,&gc58a);
 resource_ptr_table[i++]=gc58a;
 resource_load_record_alloc(0x4f,&gc58e);
 for(j=0;j<12;j++) resource_ptr_table[i++]=gc58e+((int far *)gc58e)[j]+2;
 resource_load_record_alloc(0x50,&gc584);
 for(j=0;j<4;j++) resource_ptr_table[i++]=gc584+((int far *)gc584)[j]+2;
 resource_load_record_alloc(0x51,&gc59e);
 for(j=0;j<1;j++) resource_ptr_table[i++]=gc59e+((int far *)gc59e)[j]+2;
 while(i<84) resource_ptr_table[i++]=resource_ptr_table[0];
 g720=2;
}


/* ---- F_B3D7 (original code at 0xB3D7) ---- */
extern void f03c9(), f03b4();
extern char far *ui_gfx_shadow_a;
void fb3d7(void) { resource_load_record(84); f03c9(0,0,ui_gfx_shadow_a); f03b4(0,0,320,200,0,200); }


/* ---- F_B40F (original code at 0xB40F) ---- */
/* Exact C recovery of compact descriptor expansion; far offsets are bytes. */
extern char far *ui_gfx_shadow_a,*gc592,*gc596,*resource_ptr_table[];
void level_expand_descriptors(void)
{
 register int i,j;
 i=0;
 resource_load_record(0x57);memmove(g9bfc,ui_gfx_shadow_a,0x54);
 resource_load_record(0x58);memmove(gbf66,ui_gfx_shadow_a,0x54);
 resource_load_record_alloc(0x55,&gc592);
 for(j=0;j<2;j++) resource_ptr_table[i++]=gc592+((int far *)gc592)[j]+2;
 resource_load_record_alloc(0x56,&gc596);
 for(j=0;j<12;j++) resource_ptr_table[i++]=gc596+((int far *)gc596)[j]+2;
 while(i<84) resource_ptr_table[i++]=resource_ptr_table[0];
}


/* ---- F_B4FB (original code at 0xB4FB) ---- */
extern void setmem();extern char g40d4[];fb4fb(){setmem(g40d4,672,0);g40d4[72]=3;g40d4[73]=9;g40d4[74]=13;g40d4[75]=17;g40d4[76]=6;g40d4[77]=9;g40d4[78]=16;g40d4[79]=17;g40d4[416]=15;g40d4[417]=5;g40d4[418]=47;g40d4[419]=34;g40d4[420]=12;g40d4[421]=5;g40d4[422]=44;g40d4[423]=34;}


/* ---- F_B55E (original code at 0xB55E) ---- */
extern char far *g7352, far *g7356, far *g735a, far *gbfc8;
void fb55e(void) { farfree(g7352); farfree(g7356); farfree(g735a); farfree(gbfc8); }


/* ---- F_B593 (original code at 0xB593) ---- */
extern char *gc5a8, *gc580, *gc5ac, *gc59a, *gc5a4, far *gc58a, *gc584, *gc58e, *gc59e;

void level_free_descriptor_table()
{
    farfree(gc5a8);
    farfree(gc580);
    farfree(gc5ac);
    farfree(gc59a);
    farfree(gc5a4);
    farfree(gc58a);
    farfree(gc58e);
    farfree(gc584);
    farfree(gc59e);
    g720 = 0;
}


/* ---- F_B60F (original code at 0xB60F) ---- */
extern void f03b4();extern int board_record_index;extern unsigned char gb3ae[],g9bfc[],gbf66[];struct R{char a,b;int x,y;char c,d,e;char rest[23];};fb60f(){register int x,y;int i,r,b;struct R *p;p=(struct R *)(gb3ae+1);for(i=0;i<gb3ae[0];i++,p++){if(!p->e&&p->b==board_record_index){r=(x=p->x)+g9bfc[p->c]-1;b=(y=p->y)+gbf66[p->c]-1;if(x<0)x=0;if(y<0)y=0;if(r>=0)f03b4(x,y+200,r-x+1,b-y+1,x,y);}}}


/* ---- F_B6CD (original code at 0xB6CD) ---- */
extern int hud_panel_clear(), fb60f();extern void f4eeb();extern void f4b0c();extern void f039f();
extern void menu_list_disable(void);
extern void fb3d7(void);
extern void f1ecd(void);
extern void timer_deadline_wait(void);
extern void level_expand_descriptors(void);extern int g8fe,gbc,board_record_index,g40ce,g98,g94,g96,g9a,g1776,g1784;
extern char *ui_gfx_blob;
extern char far *rect_queue_write_ptr;fb6cd(){register int i;menu_list_disable();hud_panel_clear();board_record_index=gbc=g8fe=0;g40ce=1;g94=g98=0;g96=0x1e8;g9a=160;fb3d7();level_expand_descriptors();f4eeb(0);f039f(0,0,320,200);gbc=1;i=0;do{timer_deadline_arm(24);rect_queue_write_ptr=ui_gfx_blob;fb60f();f4b0c();if(rect_queue_write_ptr!=ui_gfx_blob)f1ecd();timer_deadline_wait();if(++i==200)g1776=0;}while(i<200||g1784);}


/* ---- F_B772 (original code at 0xB772) ---- */
extern unsigned char gb3ae[];extern int near gb52f[];extern int gc588,gc5a2;struct R_B772{char a,b;int x,y;char c;char rest[25];};fb772(){register int i,j;int *q;int n;gc588=*(unsigned char *)gb52f=0;gc5a2=13;n=gb3ae[0];j=n*32+1;for(i=5;i<=11;i+=2){if(gb3ae[i*32+1]==0&&gb3ae[i*32+7]==9){q=(int *)(gb3ae+14+i*32);q[0]=(i-5)/2*3+j;q[2]=0;}}}


/* ---- F_B7F9 (original code at 0xB7F9) ---- */
/* Exact Turbo C reconstruction. Board fields use the existing workspace
 * arrays; no new storage or interior-address globals are introduced. */
extern void timer_deadline_wait(void);extern void f4e9f();extern void f4b0c();extern void f03b4();extern void f039f();
extern void fcaf1();
extern void f1ecd(void);
extern void resource_record_cache_reset();
extern void sound_stop_reset(void);
extern void sprite_draw_cursor(void);
extern void f03cc(int,int,void far *,int);
extern int gbc,g1774,g96;
extern int near gb52f[];
extern unsigned char gb6cf;
extern char far *ui_gfx_blob;
extern char far *ui_gfx_shadow_a,far *rect_queue_write_ptr,far *board_records;
void level_display_init(void)
{
 register int x,i;
 resource_load_record(0x48);gbc=0;((unsigned char near *)gb3af)[136]=1;((unsigned char near *)gb3af)[200]=1;((unsigned char near *)gb3af)[264]=1;((unsigned char near *)gb3af)[328]=1;
 sound_stop_reset();g1774=1;fcaf1(25);
 for(x=118;x<=198;x+=4) {
  timer_deadline_arm(24);f03b4(x-4,355,92,117,x-4,27);f03cc(x,27,ui_gfx_shadow_a,0);
  gb52f[33]+=4;gb52f[49]+=4;gb52f[65]+=4;gb52f[97]+=4;gb52f[129]+=4;gb52f[161]+=4;
  f4b0c();sprite_draw_cursor();f039f(x-4,27,96,117);timer_deadline_wait();
 }
 g1774=0;sound_stop_reset();sprite_draw_cursor();g96=488;f03cc(x-4,355,ui_gfx_shadow_a,0);
 f03b4(6,344,308,144,6,200);g96=159;gbc=1;gb6cf=0;resource_record_cache_reset(69);
 for(i=0;i<20;i++) {
  timer_deadline_arm(24);rect_queue_write_ptr=ui_gfx_blob;f4e9f();f4b0c();sprite_draw_cursor();f1ecd();timer_deadline_wait();
 }
 for(i=433;i<=440;i++) board_records[i]=7;
}


/* ---- F_B967 (original code at 0xB967) ---- */
extern char far *ui_gfx_blob;
extern char far *board_records, far *rect_queue_write_ptr;
extern unsigned char gb6cf;
extern void f4b0c(), f1ecd(void), f4e9f(), timer_deadline_wait();
void fb967(void) { *board_records=7; f4b0c(); f1ecd(); while(!gb6cf) { timer_deadline_arm(24); rect_queue_write_ptr=ui_gfx_blob; f4e9f(); f4b0c(); f1ecd(); timer_deadline_wait(); } }


/* ---- F_B99F (original code at 0xB99F) ---- */
/* Board movement, jumping, collision response, and redraw loop. */
extern int g8fe,gc5a2,gbc,gc588,g40ce,g1776,g1784,g96;
extern int gb6e,gb6c,gb68,gb70,g72c,g72e,g730,g732,g734,g736,g738,g73a;
extern int near g1684[];
extern unsigned char near g740[];
extern unsigned char gb3ae[];
extern struct dialog near g1670;
extern char g8bfe[],g96ee[];
extern unsigned long gb76;
extern char far *ui_gfx_blob;
extern char far *rect_queue_write_ptr,*resource_ptr_table[],*resource_stripe_table;
extern int f6b4a(),f6b1a(),face7(),fadcf(),hud_tab_next();
extern void level_display_init(void);
extern void player_select_restart_confirm();
extern int f1f91(), fb772(), shadow_bitmap_hit_test(), hud_tab_get();extern void f4e9f();
extern void fcaf1();
extern void board_redraw_view();
extern void sprite_slots_redraw();
extern int hud_scroll_move(), energy_adjust();extern void f4b0c();
extern void fb967(void);
extern void cursor_trail_arm(void);
extern void f1ecd(void);
extern void timer_deadline_wait(void);
extern void f3986();
extern void resource_record_cache_reset();
extern void board_raycast_step(void);
extern void f03cc(int,int,void far *,int);
extern void movmem();
extern void longjmp(void far *,int);
int level_run_loop(void)
{
 unsigned long deadline;
 struct gb3af_entry saved[4];
 int key,hit,last,remaining,ended,delay,hurt,once;
 register int i,direction;
 gc5a2=hurt=delay=ended=last=g8fe=0;
 once=gc588=gbc=1;
 remaining=2;
 for(i=0;i<4;i++) movmem(&gb3af[i*2+4],&saved[i],32);
 ((unsigned char near *)gb3af)[g1684[gc5a2]*32]=0;
 deadline=0;
 for(;;) {
  timer_deadline_arm(24);g40ce=direction=key=0;
  if(remaining) {
   if(((unsigned char near *)gb3af)[g1684[gc5a2]*32]) {
    gc5a2++;if(g1684[gc5a2]<0) gc5a2=0;
    ((unsigned char near *)gb3af)[g1684[gc5a2]*32]=0;
   }
  } else {
   if(!ended&&!gb3af[24].flag) {ended=1;delay=20;}
   else if(delay&&!--delay) {
    if(face7()) level_display_init();
    else {
     gbc=0;fadcf();g1776=0;while(g1784);g1776=1;
     dialog_run(&g1670);longjmp(g8bfe,1);
    }
   }
  }
  if(f6b4a()) {
   gbc=0;key=f6b1a();
   if(key==13) hud_tab_next();else if(key==27) player_select_restart_confirm();
   gbc=1;
  }
  rect_queue_write_ptr=ui_gfx_blob;board_redraw_view();f4e9f();if(g8fe) sprite_slots_redraw();
  if(gb6e) {
   g73a=0;
   if(!(f1f91(g736+28,g738+1,39)&7)) {
    g736+=g734;direction=1;
    if(g72e<=8) {if(++g72e>8) g72e=1;}
    if(gc588) {
     for(i=0;i<4;i++) movmem(&saved[i],&gb3af[i*2+4],32);
     fb772();
    }
   }
  } else if(gb6c) {
   g73a=1;
   if(!(f1f91(g736,g738+1,39)&7)) {
    g736-=g734;direction=-1;
    if(g72e<=8) {if(++g72e>8) g72e=1;}
    if(gc588) {
     for(i=0;i<4;i++) movmem(&saved[i],&gb3af[i*2+4],32);
     fb772();
    }
   }
  }
  if(g730) {
   if(!(shadow_bitmap_hit_test(g736+8,g738-1,9)&7)) {
    g738-=g740[g730];g730--;if(++g72e>11) g72e=11;
    if(!direction) g72e=10;
   } else g730=0;
  } else if(!(shadow_bitmap_hit_test(g736+8,g738+47,9)&7)) {
   if(g732) g738+=8;else {g732=1;g738+=2;}
   g72e=(direction&1)+10;
  } else if(!(shadow_bitmap_hit_test(g736+8,g738+40,9)&7)) {
   g738+=((g738+48)/8)*8-(g738+40);g732=0;g72e=(direction&1)+10;fcaf1(11);
  } else if(key==32) {
   if(hud_tab_get()==1) {
    if(!(shadow_bitmap_hit_test(g736+8,g738-1,9)&7)) {
     fcaf1(16);g730=8;g72e=9;g734=direction?8:4;
    } else goto walk;
   } else goto walk;
  } else if(gb68&&gb70&&!(shadow_bitmap_hit_test(g736+8,g738-1,9)&7)) {
   gb70=0;g730=5;g72e=9;g734=direction?8:4;fcaf1(12);
  } else {
walk:
   if(direction) {if(g72e>8) g72e=1;}else g72e=0;
   g734=4;
   if(g738<70&&g736>90&&g736<210) {fb967();gbc=0;return 1;}
  }
  if(key==32) {
   if((i=hud_tab_get())==2&&!g72c) {
     if(hud_scroll_move(-1)!=-1) {g72c=58;hurt=0;fcaf1(0);}else fcaf1(17);
   } else if(!i) {
    if(!g8fe) {cursor_trail_arm();fcaf1(20);}else fcaf1(23);
   }
  }
  if(g72c==1) g72c=0;
  f4b0c();
  if(once&&!gc588&&gb3af[12].flag) {
   gb3af[12].rest[7]=1;once=0;g96=400;f03cc(160,274,resource_ptr_table[29],0);g96=159;
  }
  if(g40ce&&last!=g40ce) hit=gb3ae[0]-g40ce;else hit=-1;
  last=g40ce;
  if(g736<8) g736=8;else if(g736>272) g736=272;
  if(g8fe) board_raycast_step();
  if(hurt) {
   hurt--;
   if(hurt>26) f03cc(g736,g738,resource_stripe_table+14828,g73a);
   else if(hurt&1) f03cc(g736,g738,resource_stripe_table+g72e*674,g73a);
  } else if(g72c) {
   f03cc(g736,g738,g96ee,0);g72c--;
   f03cc(g736,g738,resource_stripe_table+g72e*674,g73a);
  } else if(hit>4&&hit<=11) {
   f03cc(g736,g738,resource_stripe_table+14828,g73a);hurt=30;fcaf1(1);f1ecd();gbc=0;
   if(energy_adjust(-2)<=0) {f3986();return 0;}
   gbc=1;goto frame_end;
  } else f03cc(g736,g738,resource_stripe_table+g72e*674,g73a);
  if(hit==13) {
   if(gb76>deadline) {
    g730=gb3af[13].flag=0;
    if(--remaining<0) remaining=0;
    if(!remaining&&!face7()) {f1ecd();resource_record_cache_reset(69);goto frame_end;}
   }
   deadline=gb76+500;
  }
  f1ecd();
frame_end:
  timer_deadline_wait();
 }
 gbc=0;return 1;
}


/* ---- F_C0E0 (original code at 0xC0E0) ---- */
extern int g8fe,gbc,board_record_index,g40ce,g736,g738,g73a;
extern struct record3e8 g43b4[];
extern char far *board_records;extern int fb09a(), fb4fb(), level_run_loop();extern void f4eeb();extern void f039f();extern void board_resource_expand(void);extern void resource_record_cache_reset();extern void level_free_descriptor_table();
extern void fb55e(void);
extern void sprite_draw_cursor(void);
level_play(){register int r;gbc=g8fe=0;board_records=(char *)g43b4[board_record_index=1].bytes;g40ce=1;fb55e();fb09a();board_resource_expand();fb4fb();f4eeb(0);g736=16;g738=112;g73a=0;sprite_draw_cursor();f039f(8,16,304,144);resource_record_cache_reset(67);r=level_run_loop();level_free_descriptor_table();return r;}


/* ---- F_C15E (original code at 0xC15E) ---- */
extern void hud_scroll_reset(void);
extern int face7(),hud_scroll_move(),level_play(),fb6cd();
extern void menu_backdrop_paint(void);extern void fc834();extern int g9ade,g722;level_play_chapter(){hud_scroll_reset();if(!face7())hud_scroll_move(-3);else hud_scroll_move(-4);g9ade=42;g722=0;menu_backdrop_paint();if(!level_play())return 0;fb6cd();fc834();return 1;}
