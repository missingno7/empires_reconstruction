/* src/BOARDDRW.C: Board and sprite rendering.
   One translation unit; the sections below were the separate member
   sources of grouped module C_200F_2A70 and keep their original ids. */

struct CEL { char b[0x319]; };

extern void resource_load_record_into();
extern char g6f2a[][0xe2], g893c[][0x62];
extern char g6e88[][1], gb1cc[][1], g9990[][1], g7904[][1], g6ac4[][1];
extern char g6ca6[][1], g9ae0[][1], g99da[][1], g9c50[][1], g9b6e[][1];
extern char g9a5c[][1], g7400[][1], g735e[][1];
extern void far *memmove();
extern int resource_load_record();
extern char far *ui_gfx_shadow_a;                 /* DS:C5C6 offset, DS:C5C8 segment */
extern char g2380[][0x82];
extern unsigned char g9cf2[];           /* DS:9CF2 */
extern unsigned char ga6b6[];           /* DS:A6B6 */
extern unsigned char far *gc0d6[];      /* DS:C0D6 */
extern char far *malloc();
extern void movmem();
extern char far *resource_stripe_table;                 /* DS:99D2 offset, DS:99D4 segment */
extern unsigned char g96ee[];           /* DS:96EE */
extern char far *farmalloc();
extern void resource_icon_table_load();
extern void resource_scoreboard_unpack();
extern void hud_icons_load();
extern char far *g99d6;                 /* DS:99D6 offset, DS:99D8 segment */
extern int g072c, g072e, g0736, g0738, g073a;
extern void gfx_copy_rect(unsigned,unsigned,void far *,int);
extern int g736, g738;
extern void gfx_wipe_rect();
extern struct CEL cel[];
extern int  xa[], ya[];         /* DS:8BEA, DS:8BF4 */
/*@SYM _xa=0x8BEA kind=g key=storage_objects/M_2871A.phys*/
/*@SYM _ya=0x8BF4 kind=g key=storage_objects/M_28724.phys*/
extern int  g72c, g72e, g73a, g96, gbc;
extern char far *pool;                  /* DS:99D2 */
/*@SYM _pool=0x99D2 kind=g key=storage_objects/M_29502.phys*/
extern char far *src, far *dst;         /* DS:C5CA -> DS:40C4 */
/*@SYM _src=0xC5CA kind=g key=storage_objects/M_2C0FA.phys*/
/*@SYM _dst=0x40C4 kind=g key=storage_objects/M_23BF4.phys*/
extern void sound_stop_reset(void);                    /* CB48 */
/*@SYM _sound_stop_reset=0xCB48 kind=f key=functions/F_CB48.entry*/
extern void fcaf1(int n);                   /* CAF1 */
/*@SYM _fcaf1=0xCAF1 kind=f key=functions/F_CAF1.entry*/
extern void board_redraw_view(void);                    /* 22B1 */
/*@SYM _board_redraw_view=0x22B1 kind=f key=functions/F_22B1.entry*/
extern void timer_deadline_arm(int n);               /* 6C57 */
/*@SYM _timer_deadline_arm=0x6C57 kind=f key=functions/F_6C57.entry*/
extern void blit(int x, int y, char far *s);                    /* 03C9 */
/*@SYM _blit=0x03C9 kind=f key=functions/F_03C9.entry*/
extern void wipe(int x, int y, int w, int h, int x2, int y2);   /* 03B4 */
/*@SYM _wipe=0x03B4 kind=f key=functions/F_03B4.entry*/
extern void copy(int x, int y, char far *s, int n);             /* 03CC */
/*@SYM _copy=0x03CC kind=f key=functions/F_03CC.entry*/
extern void f1ecd(void);                    /* 1ECD */
/*@SYM _f1ecd=0x1ECD kind=f key=functions/F_1ECD.entry*/
extern void timer_deadline_wait(void);                    /* 6C6F */
/*@SYM _timer_deadline_wait=0x6C6F kind=f key=functions/F_6C6F.entry*/
extern void box(int x, int y, int w, int h);                    /* 039F */
/*@SYM _box=0x039F kind=f key=functions/F_039F.entry*/
extern unsigned char far *board_records;
extern int point_in_hotspot_rect();
extern char far *record_table_root;
extern char far *record_field_skip_n();

/* ---- F_200F (original code at 0x200F) ---- */
void resource_icon_table_load()
{
    int i;
    for (i = 0; i < 4; ++i)
        resource_load_record_into(i + 5, g6f2a[i]);
    for (i = 0; i < 7; ++i)
        resource_load_record_into(i + 0xa, g893c[i]);
    resource_load_record_into(9, g6e88[0]);
    resource_load_record_into(0x11, gb1cc[0]);
    resource_load_record_into(0x12, g9990[0]);
    resource_load_record_into(0x2e, g7904[0]);
    resource_load_record_into(0x2f, g6ac4[0]);
    resource_load_record_into(0x30, g6ca6[0]);
    resource_load_record_into(0x27, g9ae0[0]);
    resource_load_record_into(0x28, g99da[0]);
    resource_load_record_into(0x29, g9c50[0]);
    resource_load_record_into(0x2a, g9b6e[0]);
    resource_load_record_into(0x2b, g9a5c[0]);
    resource_load_record_into(0x2c, g7400[0]);
    resource_load_record_into(0x2d, g735e[0]);
}


/* ---- F_2119 (original code at 0x2119) ---- */
void resource_scoreboard_unpack()
{
    char far *d;
    char far *s;
    int i;
    d = g2380[0];
    s = ui_gfx_shadow_a + 2;
    resource_load_record(0x26);
    for (i = 0; i < 8; ++i) {
        memmove(d, s, 0x82);
        d += 0x82;
        s += 0x84;
        memmove(d, s, 0x62);
        d += 0x62;
        s += 0x64;
        memmove(d, s, 0x92);
        d += 0x92;
        s += 0x94;
    }
}


/* ---- F_21A9 (original code at 0x21A9) ---- */
/* F_21A9 -- load the two boot sprite sheets and publish their far addresses
   into the DS:C0D6 table F_6CA6 indexes.  An array NAME assigned to a far
   pointer stores `ds` into the segment half and the offset CONSTANT into the
   other; the constant index folds to a direct displacement (rule 2). */
void sprite_load_boot_sheets()
{
    resource_load_record_into(0, g9cf2);
    resource_load_record_into(1, ga6b6);
    gc0d6[0] = g9cf2;
    gc0d6[1] = ga6b6;
}


/* ---- F_21DB (original code at 0x21DB) ---- */
/* F_21DB -- read record 4 into the staging block, allocate 15,502 bytes and
   copy 23 stripes of 674 bytes out of the staging block (676-byte pitch, two
   header bytes skipped) into it, then one more stripe into the DS:96EE
   buffer.  No frame: no parameter and no local (rule 11); the loop counter is
   the one register variable. */
void resource_stripe_table_load()
{
    register int i;

    resource_load_record(4);
    resource_stripe_table = malloc(0x3c8e);
    for (i = 0; i < 23; i++)
        movmem(ui_gfx_shadow_a + i * 676 + 2, resource_stripe_table + i * 674, 674);
    movmem(ui_gfx_shadow_a + 0x3cbe, g96ee, 674);
}


/* ---- F_224C (original code at 0x224C) ---- */
/* F_224C -- allocate the 0x55F0-byte arena, keep the far pointer at DS:99D6
   and run the three initialisers over it.  `xor dx,dx` before the push makes
   the size one 32-bit argument, not two ints, and the result comes back in
   DX:AX -- farmalloc, CC.LIB at IP 0E9B4h (substrate/LIB_FMALLOC.json). */
void hud_arena_init()
{
    g99d6 = farmalloc(0x55f0L);
    resource_icon_table_load();
    resource_scoreboard_unpack();
    hud_icons_load();
}


/* ---- F_2269 (original code at 0x2269) ---- */
/* F_2269 -- blit the cursor sprite, and the pending one first.  gc99d2 is a
   far pointer into the 0x2a2-byte sprite table. */
void sprite_draw_cursor(void)
{
    if (g072c) {
        gfx_copy_rect(g0736, g0738, g96ee, 0);
        g072c--;
    }
    gfx_copy_rect(g0736, g0738, resource_stripe_table + g072e * 0x2a2, g073a);
}


/* ---- F_22B1 (original code at 0x22B1) ---- */
/* F_22B1 -- clamp the view window to the board and blit it.  Plain C.
   si/di are the two register variables, [bp-4] and [bp-2] the two ints. */
void board_redraw_view(void)
{
    int w4, w2;
    register int x, y;

    if (g736 < 8) {
        x = 8;
        w4 = 0x28;
    } else if (g736 + 0x27 > 0x137) {
        x = g736;
        w4 = 0x138 - g736;
    } else {
        x = g736;
        w4 = 0x28;
    }
    if (g738 < 0x10) {
        y = 0x10;
        w2 = 0x28;
    } else if (g738 + 0x27 > 0x9f) {
        y = g738;
        w2 = 0xa0 - g738;
    } else {
        y = g738;
        w2 = 0x28;
    }
    gfx_wipe_rect(x, y + 0xb8, w4, w2, x, y);
}


/* ---- F_233E (original code at 0x233E) ---- */
void board_scan_wipe_effect(register int i)
{
    register int k;

    sound_stop_reset();
    g72c = 0;
    fcaf1(0x0d);
    g96 = 0x190;
    board_redraw_view();
    gbc = 1;
    for (k = 1; k < 5; k++) {
        timer_deadline_arm(0x18);
        dst = src;
        blit(xa[i], ya[i] + 0xb8, (char far *)&cel[k]);
        wipe(xa[i], ya[i] + 0xb8, 0x2e, 0x21, xa[i], ya[i]);
        copy(g736, g738, pool + g72e * 0x2a2, g73a);
        f1ecd();
        timer_deadline_wait();
    }
    board_redraw_view();
    for (k = 12; k < 16; k++) {
        timer_deadline_arm(0x18);
        dst = src;
        wipe(xa[i], ya[i] + 0xb8, 0x2e, 0x28, xa[i], ya[i]);
        board_redraw_view();
        copy(xa[i] + 4, ya[i], pool + k * 0x2a2, g73a);
        f1ecd();
        timer_deadline_wait();
    }
    gbc = 0;
    fcaf1(0x0d);
    for (k = 3; k >= 0; k--) {
        timer_deadline_arm(0x18);
        blit(xa[i], ya[i], (char far *)&cel[k]);
        box(xa[i], ya[i], 0x2e, 0x21);
        timer_deadline_wait();
    }
    g96 = 0x9f;
}


/* ---- F_250C (original code at 0x250C) ---- */
f250c(i) int i;{unsigned char a;unsigned char *p;p=board_records+i*3+0x2ac;if(a=*p){if(a&=15){*p=(*p&0xf0)|(6-a);if(*p&15)*p^=32;}else *p=(*p&0xf0)|6;}}


/* ---- F_257D (original code at 0x257D) ---- */
int point_in_hotspot_rect(int x, int y)
{
    if (x >= g736 && x <= g736 + 0x1f && y >= g738 && y <= g738 + 0x27)
        return 1;
    return 0;
}


/* ---- F_25B3 (original code at 0x25B3) ---- */
/* Update the ten moving board records and redraw the changed cells. */
void board_update_moving_records(void)
{
 int i,index;
 unsigned char flag;
 unsigned char far *p;
 register unsigned x,y;
 p=board_records+684;
 for(i=0;i<10;i++,p+=3) {
  if((flag=*p)&15) {
   if(!(--*p&15)) *p^=32;
   /* See F_28AC: this intermediate preserves Turbo C's AX:DX extension. */
   x=(unsigned)(char far *)p[1];x<<=1;
   y=(unsigned)(char far *)p[2];
   index=(x>>3)+((y>>3)-2)*38-1;
   y+=180;x-=4;
   if(flag&128) {
    if(flag&32) {
     if(point_in_hotspot_rect(x+4,y-192)) *p=flag;
     else {
      gfx_wipe_rect(x,y+144,16,56,x,y);
      y-=8;p[2]-=8;
      g96=359;gfx_copy_rect(x,y,g6ca6,0);g96=159;
      gfx_wipe_rect(x,y,16,64,x,y-184);
      board_records[index-38]=7;board_records[index+190]=0;
     }
    } else {
     if(point_in_hotspot_rect(x+4,y-120)) *p=flag;
     else {
      gfx_wipe_rect(x,y+144,16,56,x,y);
      p[2]+=8;
      g96=359;gfx_copy_rect(x,y+8,g6ca6,0);g96=159;
      gfx_wipe_rect(x,y,16,64,x,y-184);
      board_records[index+228]=7;board_records[index]=0;
     }
    }
   } else {
    if(flag&32) {
     if(point_in_hotspot_rect(x-8,y-180)) *p=flag;
     else {
      gfx_wipe_rect(x,y+144,56,16,x,y);
      x-=8;p[1]-=4;
      g96=359;gfx_copy_rect(x,y,g6ac4,0);g96=159;
      gfx_wipe_rect(x,y,64,16,x,y-184);
      board_records[index-1]=7;board_records[index+5]=0;
     }
    } else {
     if(point_in_hotspot_rect(x+64,y-180)) *p=flag;
     else {
      gfx_wipe_rect(x,y+144,56,16,x,y);
      p[1]+=4;
      g96=359;gfx_copy_rect(x+8,y,g6ac4,0);g96=159;
      gfx_wipe_rect(x,y,64,16,x,y-184);
      board_records[index+6]=7;board_records[index]=0;
     }
    }
   }
  }
 }
}


/* ---- F_28AC (original code at 0x28AC) ---- */
/* Exact Turbo C reconstruction of the ten-record board drawing loop.
 * The far-pointer intermediate retains the historical zero extension into
 * AX:DX before the low word is assigned to a coordinate. It is never
 * dereferenced. This compiler-specific expression is not provenance proof.
 */
void board_mark_record_cells(void)
{
 int i,index,stride,j;
 unsigned char flag;
 unsigned char far *p;
 register unsigned x,y;
 p=board_records+684;
 for(i=0;i<10;i++,p+=3) {
  if(flag=*p) {
   x=(unsigned)(char far *)p[1];x<<=1;
   y=(unsigned)(char far *)p[2];
   index=(x>>3)+((y>>3)-2)*38-1;
   y+=180;x-=4;
   if(flag&128) {gfx_copy_rect(x,y,g6ca6,0);stride=38;}
   else {gfx_copy_rect(x,y,g6ac4,0);stride=1;}
   for(j=0;j<6;j++,index+=stride) board_records[index]=7;
  }
 }
}


/* ---- F_2986 (original code at 0x2986) ---- */
extern int g00bc;

void f2986(p)
unsigned char far *p;
{
    int saved;
    int n;
    int off;
    register int di;
    register int i;

    saved = g00bc;
    di = p[0] * 2;
    n = p[1];
    gfx_copy_rect(di, n + 0xb8, gb1cc[0], 0);
    g00bc = 0;
    i = p[4] + 5;
    while (i < 10) {
        if (p[i] == 0)
            break;
        off = (i - 5) * 10 + di + 6;
        gfx_copy_rect(off, n + 0xb9, g893c[p[i] - 1], 0);
        ++i;
    }
    g00bc = saved;
}


/* ---- F_2A2D (original code at 0x2A2D) ---- */
char far *record_field_skip_n(int n)
{
    unsigned char far *p;
    int i;

    p = (unsigned char far *) (record_table_root + (*record_table_root) * 4 + 2);
    for (i = 0; i < n; i++)
        p += *p;
    return (char far *) p;
}


/* ---- F_2A70 (original code at 0x2A70) ---- */
/* Walk the nested count-prefixed tables at record_table_root and return the near address of the fourth level. */
unsigned char near *f2a70(){unsigned char *p;p=record_table_root+*record_table_root*4+1;p=record_field_skip_n(*p);p+=*p*3+1;p+=*p*12+1;return (unsigned char near *)(p+*p*3+1);}
