/* src/BOARD.C: Board drawing, painting and the unit script interpreter.
   One translation unit; the sections below were the separate member
   sources of grouped module C_200F_3986 and keep their original ids. */

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
extern void gfx_copy_rect(int,int,void far *,int);
extern int cursor_x, cursor_y;
extern void gfx_wipe_rect();
extern struct CEL cel[];
extern int  xa[], ya[];         /* DS:8BEA, DS:8BF4 */
/*@SYM _xa=0x8BEA kind=g key=storage_objects/M_2871A.phys*/
/*@SYM _ya=0x8BF4 kind=g key=storage_objects/M_28724.phys*/
extern int  hud_scroll_cooldown_ticks, g72e, cursor_facing_left, g96, gbc;
extern char far *pool;                  /* DS:99D2 */
/*@SYM _pool=0x99D2 kind=g key=storage_objects/M_29502.phys*/
extern char far *src, far *dst;         /* DS:C5CA -> DS:40C4 */
/*@SYM _src=0xC5CA kind=g key=storage_objects/M_2C0FA.phys*/
/*@SYM _dst=0x40C4 kind=g key=storage_objects/M_23BF4.phys*/
extern void sound_stop_reset(void);                    /* CB48 */
/*@SYM _sound_stop_reset=0xCB48 kind=f key=functions/F_CB48.entry*/
extern void stream_control_block_arm(int n);                   /* CAF1 */
/*@SYM _stream_control_block_arm=0xCAF1 kind=f key=functions/F_CAF1.entry*/
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
extern void rect_queue_flush(void);                    /* 1ECD */
/*@SYM _rect_queue_flush=0x1ECD kind=f key=functions/F_1ECD.entry*/
extern void timer_deadline_wait(void);                    /* 6C6F */
/*@SYM _timer_deadline_wait=0x6C6F kind=f key=functions/F_6C6F.entry*/
extern void box(int x, int y, int w, int h);                    /* 039F */
/*@SYM _box=0x039F kind=f key=functions/F_039F.entry*/
extern char far *board_records;
extern int point_in_hotspot_rect();
extern char far *record_table_root;
extern char far *record_field_skip_n();
extern void f2986();
extern int board_record_index;
extern char s9b6e[], s9ae0[], s9a5c[], s99da[];
extern unsigned char far *g96ea;

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

    if (cursor_x < 8) {
        x = 8;
        w4 = 0x28;
    } else if (cursor_x + 0x27 > 0x137) {
        x = cursor_x;
        w4 = 0x138 - cursor_x;
    } else {
        x = cursor_x;
        w4 = 0x28;
    }
    if (cursor_y < 0x10) {
        y = 0x10;
        w2 = 0x28;
    } else if (cursor_y + 0x27 > 0x9f) {
        y = cursor_y;
        w2 = 0xa0 - cursor_y;
    } else {
        y = cursor_y;
        w2 = 0x28;
    }
    gfx_wipe_rect(x, y + 0xb8, w4, w2, x, y);
}


/* ---- F_233E (original code at 0x233E) ---- */
void board_scan_wipe_effect(register int i)
{
    register int k;

    sound_stop_reset();
    hud_scroll_cooldown_ticks = 0;
    stream_control_block_arm(0x0d);
    g96 = 0x190;
    board_redraw_view();
    gbc = 1;
    for (k = 1; k < 5; k++) {
        timer_deadline_arm(0x18);
        dst = src;
        blit(xa[i], ya[i] + 0xb8, (char far *)&cel[k]);
        wipe(xa[i], ya[i] + 0xb8, 0x2e, 0x21, xa[i], ya[i]);
        copy(cursor_x, cursor_y, pool + g72e * 0x2a2, cursor_facing_left);
        rect_queue_flush();
        timer_deadline_wait();
    }
    board_redraw_view();
    for (k = 12; k < 16; k++) {
        timer_deadline_arm(0x18);
        dst = src;
        wipe(xa[i], ya[i] + 0xb8, 0x2e, 0x28, xa[i], ya[i]);
        board_redraw_view();
        copy(xa[i] + 4, ya[i], pool + k * 0x2a2, cursor_facing_left);
        rect_queue_flush();
        timer_deadline_wait();
    }
    gbc = 0;
    stream_control_block_arm(0x0d);
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
    if (x >= cursor_x && x <= cursor_x + 0x1f && y >= cursor_y && y <= cursor_y + 0x27)
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
void f2986(p)
unsigned char far *p;
{
    int saved;
    int n;
    int off;
    register int di;
    register int i;

    saved = gbc;
    di = p[0] * 2;
    n = p[1];
    gfx_copy_rect(di, n + 0xb8, gb1cc[0], 0);
    gbc = 0;
    i = p[4] + 5;
    while (i < 10) {
        if (p[i] == 0)
            break;
        off = (i - 5) * 10 + di + 6;
        gfx_copy_rect(off, n + 0xb9, g893c[p[i] - 1], 0);
        ++i;
    }
    gbc = saved;
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
unsigned char near *record_table_level4_ptr(){unsigned char *p;p=record_table_root+*record_table_root*4+1;p=record_field_skip_n(*p);p+=*p*3+1;p+=*p*12+1;return (unsigned char near *)(p+*p*3+1);}


/* ---- F_2AE2 (original code at 0x2AE2) ---- */
/* F_2AE2 -- the board redraw.  Plain C.  The module is on the TASM path:
   the two  83 E3 3F  (and bx,3fh) at 2C31 and 2D78 are the short AND form
   TASM picks and TCC's own writer does not. */
extern int value_parity();
/* alternate view: untyped extern void gfx_copy_rect(); vs typed extern void gfx_copy_rect(int,int,void far *,int); at top */
extern void gfx_copy_rect();
extern void gfx_blit_bitmap();
extern int sprite_table_queue_draws(), draw_queue_render_highlighted(), draw_queue_reset();
extern void board_mark_record_cells(void);
extern void draw_queue_append(char, int, int, int, int);
extern unsigned char near *record_table_level4_ptr();

extern unsigned char far *icon_record_list_ptr;
extern unsigned char far *g96e6;
extern char far *g40d0;
extern int g94, g722, g73c, campaign_round_node_cursor, gb07a;
extern int w8bea[], w8bf4[];
extern unsigned char b4377[], b437a[], b4380[], b4386[];
extern char s8c12[], s79bf[], s7400[], s735e[];
extern char s9c50[], s6e88[];
extern char far *a72b2[];
extern char a74a2[][0xbb], a6f2a[][0xe2], a893c[][0x62];

void board_redraw_paint()
{
    unsigned char c;
    unsigned char far *p;
    unsigned char far *q;
    int w8;
    int n;
    int w4;
    int w2;
    register int i, j;

    if (campaign_round_node_cursor == 0x2a)
        return;
    record_table_root = (char far *) (board_records + 0x2ca);
    gfx_blit_bitmap(8, 0xc8, s8c12);
    gfx_blit_bitmap(0x54, 0xc8, s8c12);
    gfx_blit_bitmap(0xa0, 0xc8, s8c12);
    gfx_blit_bitmap(0xec, 0xc8, s8c12);
    gfx_blit_bitmap(8, 0x110, s8c12);
    gfx_blit_bitmap(0x54, 0x110, s8c12);
    gfx_blit_bitmap(0xa0, 0x110, s8c12);
    gfx_blit_bitmap(0xec, 0x110, s8c12);
    g96 = 0x190;
    if (value_parity(campaign_round_node_cursor) != 0) {
        resource_load_record(g73c + 0x101e);
        gfx_copy_rect(8, 0xc8, ui_gfx_shadow_a, 0);
    }
    n = *(q = record_table_level4_ptr());
    p = q + 1;
    for (i = 0; i < n; i++, p += 3)
        if (p[2] >= 0x80) {
            j = p[0] * 2;
            w8 = p[1];
            gfx_copy_rect(j, w8 + 0xb8, a72b2[p[2] & 0x3f], p[2] & 0x40);
        }
    icon_record_list_ptr = p;
    g94 = 0xc8;
    i = 0;
    w8 = i;
    for (; i < 0x12; i++)
        for (j = 0; j < 0x26; j++) {
            if ((c = board_records[w8]) & 7) {
                c = (c & 7) - 1;
                if (c < 6)
                    gfx_copy_rect(j * 8 + 4, i * 8 + 0xc4, a74a2[c], 0);
            } else if (c & 0x80) {
                c = (c & 0x70) >> 4;
                if (c != 0)
                    gfx_copy_rect(j * 8 + 8, i * 8 + 0xc8, a6f2a[c - 1], 0);
            }
            w8++;
        }
    g94 = 0x10;
    p = q + 1;
    for (i = 0; i < n; i++, p += 3)
        if (p[2] < 0x80) {
            j = p[0] * 2;
            w8 = p[1];
            gfx_copy_rect(j, w8 + 0xb8, a72b2[p[2] & 0x3f], p[2] & 0x40);
        }
    if (gb07a != 0) {
        if (b4377[0] == board_record_index) {
            g722 = 1;
            w8bea[0] = b4377[1];
            w8bea[0] <<= 1;
            w8bf4[0] = b4377[2];
        } else {
            g722 = 0;
        }
    }
    for (i = 0; i < g722; i++)
        gfx_blit_bitmap(w8bea[i], w8bf4[i] + 0xb8, s79bf);
    if (*record_table_root != 0)
        draw_queue_render_highlighted();
    gfx_wipe_rect(0, 0xc8, 0x140, 0x90, 0, 0x158);
    g96 = 0x190;
    board_mark_record_cells();
    draw_queue_reset();
    for (i = 0; i < 6; i++)
        if (board_record_index + 1 == b437a[i]) {
            gfx_copy_rect((w8 = b4380[i]) * 2, (j = b4386[i]) + 0xb8, s7400, 0);
            draw_queue_append(i + 1, w8, j, 8, 0x10);
        }
    if (((unsigned char far *)board_records)[0x3e7] == board_record_index + 1) {
        gfx_copy_rect((w8 = ((unsigned char far *)board_records)[0x3e5]) * 2, (j = ((unsigned char far *)board_records)[0x3e6]) + 0xb8, s735e, 0);
        draw_queue_append(7, w8, j, 8, 0x10);
    }
    p = (unsigned char far *) (record_table_root + *record_table_root * 4 + 1);
    n = *p;
    i = 0;
    p++;
    for (; i < n; i++, p += p[0]) {
        j = p[2];
        w8 = p[3];
        if ((w4 = p[1]) > 2) {
            draw_queue_append(i + 8, j, w8, 4, 8);
        } else if (w4 == 0) {
            if (p[4] != 0)
                gfx_copy_rect(j * 2, w8 + 0xb8, s9b6e, 0);
            else
                gfx_copy_rect(j * 2, w8 + 0xb8, s9ae0, 0);
            draw_queue_append(i + 8, j + 6, w8 + 3, 2, 6);
        } else if (w4 == 1) {
            if (p[4] != 0)
                gfx_copy_rect(j * 2, w8 + 0xb8, s9a5c, 0);
            else
                gfx_copy_rect(j * 2, w8 + 0xb8, s99da, 0);
            draw_queue_append(i + 8, j + 3, w8, 7, 7);
        } else {
            gfx_copy_rect(j * 2, w8 + 0xb8, s9c50, 0);
            draw_queue_append(i + 8, j, w8, 8, 0x10);
        }
    }
    n = *(g96e6 = p);
    i = 0;
    p++;
    for (; i < n; i++, p += 3) {
        j = p[0] * 2;
        w8 = p[1];
        gfx_copy_rect(j, w8 + 0xb8, s6e88, 0);
        gfx_copy_rect(j + 4, w8 + 0xb8, a893c[p[2]], 0);
        draw_queue_append(p[2] + 0x20, p[0] + 1, w8 + 4, 6, 0xa);
    }
    n = *(g96ea = p);
    i = 0;
    p++;
    for (; i < n; i++, p += 0xc) {
        f2986(p);
        w2 = p[0] / 4 + (p[1] / 8 - 1) * 0x26 - 1;
        for (w8 = 0; w8 < 6; w8++, w2++)
            board_records[w2] = board_records[w2 + 0x26] = 7;
    }
    if (*(g40d0 = p) != 0)
        sprite_table_queue_draws();
    g96 = 0x9f;
}

#include "LAYOUT.H"
#include "R3E8.H"
#include "GB3AF.H"
#include "C470.H"
#include "VIDEO.H"

extern struct record3e8 g43b4[];

extern char far *ui_gfx_blob, far *rect_queue_write_ptr;
/* alternate view: untyped extern void timer_deadline_arm(); vs typed extern void timer_deadline_arm(int n); at top */
extern void timer_deadline_arm();
extern void sprite_draw_cursor(void);
extern void timer_wait_ticks();
extern int puzzle_run();
extern void board_actors_draw(int n);
extern void hud_panel_open();
extern void board_redraw_paint(void);
extern int g40ce;
extern int f250c(), f32fa(), sprite_record_adjust_draw();
/* alternate view: untyped extern void stream_control_block_arm(); vs typed extern void stream_control_block_arm(int n); at top */
extern void stream_control_block_arm();
extern int g73e;
extern void anim_step_loop(int x, int y, int w, int h, int x2, int y2);
extern void energy_set(int n);
extern int confirm_quit_dialog(void);
extern char game_abort_jmpbuf[];
extern void longjmp(void far *s, int n);

/* ---- F_31C4 (original code at 0x31C4) ---- */
/* F_31C4 -- scroll the board into view, redraw the frame, scroll it out.
   si is the row cursor in both loops; ui_gfx_blob is copied into rect_queue_write_ptr as a far
   pointer (les bx / mov es / mov bx). */
void board_scroll_transition(void)
{
    register int y;

    if (g072e <= 8) {
        rect_queue_write_ptr = ui_gfx_blob;
        for (y = 0x10; y < 0x14; y++) {
            timer_deadline_arm(0x30);
            g072e = y;
            board_redraw_view();
            sprite_draw_cursor();
            rect_queue_flush();
            timer_deadline_wait();
        }
        timer_wait_ticks(0x30);
    }
    gbc = 0;
    sound_stop_reset();
    puzzle_run();
    board_redraw_paint();
    gfx_wipe_rect(8, 0xc8, 0x130, 0x90, 8, 0x10);
    board_actors_draw(0);
    sprite_draw_cursor();
    gfx_box(8, 0x10, 0x130, 0x90);
    hud_panel_open();
    gbc = 1;
    rect_queue_write_ptr = ui_gfx_blob;
    if (g072e == 0x13) {
        for (y = 0x13; y >= 0x10; y--) {
            timer_deadline_arm(0x30);
            g072e = y;
            board_redraw_view();
            sprite_draw_cursor();
            rect_queue_flush();
            timer_deadline_wait();
        }
        g072e = 0;
        timer_wait_ticks(0x30);
    }
}


/* ---- F_329F (original code at 0x329F) ---- */
void board_record_index_select(void)
{
    board_records = (char far *) &g43b4[board_record_index];
    board_redraw_paint();
    dst = src;
    wipe(8, 0xc8, 0x130, 0x90, 8, 0x10);
    g40ce = gbc = 0;
    board_actors_draw(0);
    gbc = 1;
}


/* ---- F_32FA (original code at 0x32FA) ---- */
f32fa(a) int a;{register int i;int j;int x,y,n;unsigned char *p;p=record_table_root+a*4+1;p[3]=(p[3]+4)&7;n=p[2]+2;x=p[0];y=p[1];j=(x+2)/4+((y+4)/8-2)*38-1;for(i=0;i<n;i++,j++)board_records[j]^=16;}


/* ---- F_338A (original code at 0x338A) ---- */
/* F_338A -- dispatch one scripted event stream and animate its unit. */
void board_run_unit_script(unsigned char far *s)
{
    int w, v, u, i, n, k;
    unsigned char c;
    unsigned char far *r;
    unsigned char far *q;
    register int x, y;

    n = *s;
    if (s[1] < 2) {
        x = s[2];
        x <<= 1;
        y = s[3];
        if (s[1] != 0) {
            x = s[2];
            x <<= 1;
            y = s[3];
            gfx_wipe_rect(x, y + 0x148, 0x18, 8, x, y + 0xb8);
            g96 = 0x167;
            if (s[4] ^= 1)
                gfx_copy_rect(x, y + 0xb8, s9a5c, 0);
            else
                gfx_copy_rect(x, y + 0xb8, s99da, 0);
            g96 = 0x9f;
            gfx_wipe_rect(x, y + 0xb8, 0x18, 8, x, y);
        } else {
            x = s[2];
            x <<= 1;
            y = s[3];
            gfx_wipe_rect(x, y + 0x148, 0x18, 9, x, y + 0xb8);
            g96 = 0x167;
            if (s[4] ^= 1)
                gfx_copy_rect(x, y + 0xb8, s9b6e, 0);
            else
                gfx_copy_rect(x, y + 0xb8, s9ae0, 0);
            g96 = 0x9f;
            gfx_wipe_rect(x, y + 0xb8, 0x18, 9, x, y);
        }
        stream_control_block_arm(8);
    } else if (s[1] == 2)
        stream_control_block_arm(0x16);
    s += i = 5;
    for (; i < n; i++, s++) {
        c = *s;
        if ((c & 0x80) == 0) {
            if (c & 0x10)
                f32fa(c & 0xf);
            else if (c & 0x40)
                sprite_record_adjust_draw(c & 0xf);
            else
                f250c(c);
        } else if ((c & 0x30) == 0) {
            i++;
            s++;
            r = g43b4[*s].bytes + (c & 0x7f) * 3 + 0x2ac;
            c = *r;
            *r &= 0xf0;
            *r ^= 0x20;
            if (c & 0x80) {
                if (c & 0x20)
                    r[2] -= 0x30;
                else
                    r[2] += 0x30;
            } else {
                if (c & 0x20)
                    r[1] -= 0x18;
                else
                    r[1] += 0x18;
            }
        } else if (c & 0x10) {
            i++;
            s++;
            r = (q = g43b4[*s].bytes) + (c & 0xf) * 4 + 0x2cb;
            /* r[3] = -r[3], negated at WORD width without an extension: every
               C spelling (28 standalone forms, 8 in context, 2026-09-21,
               docs/history/probes/neg-ax-forms.C) folds a byte-lvalue negation
               to `neg al`, and every route to `neg ax` (int/static/register
               temp) materialises `mov ah,0` plus a spill/reload.  The original
               relies on AH still being zero from the `and ax,0Fh` three
               statements earlier, which only the author knew: hand-written
               inline asm inside this otherwise compiled function. */
            asm les bx,r
            asm mov al,es:[bx+3]
            asm neg ax
            asm les bx,r
            asm mov es:[bx+3],al
            c = r[2] + 2;
            w = r[0];
            v = r[1];
            u = (w + 2) / 4 + ((v + 4) / 8 - 2) * 0x26 - 1;
            for (k = 0; c > k; k++, u++)
                q[u] ^= 0x10;
        } else if (c & 0x20) {
            i++;
            s++;
            c = *s;
            ((unsigned char near *) actor_state_table)[c << 5] = 0;
        }
    }
}


/* ---- F_36F0 (original code at 0x36F0) ---- */
/* F_36F0 -- advance every queued 12-byte move record by one step: either
   finish the move (swap the from/to cells, repaint, re-mark the board) or
   redraw the unit at its current cell. */
void board_advance_unit_moves(int a)
{
    int n;
    int i;
    unsigned char far *p;
    unsigned char s;
    unsigned char cx;
    unsigned char cy;
    register int u, k;

    g96 = 0x190;
    stream_control_block_arm(0xa);
    a++;
    n = *g96ea;
    p = g96ea + 1;
    for (i = 0; i < n; i++, p += 0xc) {
        s = p[4];
        cx = p[0];
        cy = p[1];
        if (p[s + 5] != a) {
            if (s != 0) {
                p[4] = 0;
                f2986(p);
                gfx_wipe_rect(cx * 2, cy + 0xb8, 0x38, 0x10, cx * 2, cy);
            }
        } else {
            p[4] = ++s;
            if (s == 5 || p[s + 5] == 0) {
                p[4] = 0;
                u = p[0] / 4 + (p[1] / 8 - 1) * 0x26 - 1;
                for (k = 0; k < 6; k++, u++)
                    board_records[u] = board_records[u + 0x26] = 0;
                p[0] = p[2];
                p[1] = p[3];
                p[2] = cx;
                p[3] = cy;
                gfx_wipe_rect(cx * 2, cy + 0x148, 0x38, 0x10, cx * 2, cy);
                gfx_wipe_rect(cx * 2, cy + 0x148, 0x38, 0x10, cx * 2, cy + 0xb8);
                f2986(p);
                cx = p[0];
                cy = p[1];
                gfx_wipe_rect(cx * 2, cy + 0xb8, 0x38, 0x10, cx * 2, cy);
                u = cx / 4 + (cy / 8 - 1) * 0x26 - 1;
                for (k = 0; k < 6; k++, u++)
                    board_records[u] = board_records[u + 0x26] = 7;
            } else {
                f2986(p);
                gfx_wipe_rect(cx * 2, cy + 0xb8, 0x38, 0x10, cx * 2, cy);
            }
        }
    }
    g96 = 0x9f;
}


/* ---- F_3986 (original code at 0x3986) ---- */
void board_record_complete(void)
{
    int si, di, w, h;

    g73e = board_record_index;
    g96 = 0x190;
    board_actors_draw(0xb8);
    g96 = 0x9f;

    if (cursor_x < 8) {
        si = 8;
        w = 0x28;
    } else if (cursor_x + 0x27 > 0x137) {
        si = cursor_x;
        w = 0x138 - cursor_x;
    } else {
        si = cursor_x;
        w = 0x28;
    }

    if (cursor_y < 0x10) {
        di = 0x10;
        h = 0x28;
    } else if (cursor_y + 0x27 > 0x9f) {
        di = cursor_y;
        h = 0xa0 - cursor_y;
    } else {
        di = cursor_y;
        h = 0x28;
    }

    anim_step_loop(si, di + 0xb8, w, h, si, di);

    slot_table[current_slot].value++;
    energy_set(slot_table[current_slot].state = 4);
    if (confirm_quit_dialog())
        longjmp((char far *) game_abort_jmpbuf, 2);
}
