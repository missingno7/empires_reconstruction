/* src/HITTEST.C: Tile and sprite hit testing.
   One translation unit; the sections below were the separate member
   sources of grouped module C_5A3B_6021 and keep their original ids. */

struct B3 { unsigned char a, b, c; };

extern int g736, g738, g73a;
extern int gc04e, gc0b0, gc0b6, gc0b8, gc0c0;
extern int gc050[], gc080[];
extern int f020f(), rect_table_hit_id(), f03d2(), board_raycast_hit_test();
extern void fcaf1();
extern void gfx_color_select(int n);
extern char far *record_field_skip_n();

extern int tx[24];                      /* DS:C050 */
/*@SYM _tx=0xC050 kind=g key=storage_objects/REGION_2BB80.phys*/
extern int ty[24];                      /* DS:C080 */
/*@SYM _ty=0xC080 kind=g key=storage_objects/REGION_2BBB0.phys*/
extern int wig[24][12];                 /* DS:0900 */
extern int g8fe;                        /* DS:08FE */
extern int gc046, gc048, gc04a, gc04c;
extern int gc0b2, gc0b4, gc0be, gc0c2;
extern char far *gc0ba;                 /* DS:C0BA */
extern char far *gc0c4;                 /* DS:C0C4 */
extern char far *vram;                  /* DS:BFBC */
/*@SYM _vram=0xBFBC kind=g key=storage_objects/M_2BAEC.phys*/
extern char far *sprbase;               /* DS:BFC8 */
extern char far *edge;                  /* DS:40C4 */
/*@SYM _edge=0x40C4 kind=g key=storage_objects/M_23BF4.phys*/
extern char far *objtab;                /* DS:40D0 */
/*@SYM _objtab=0x40D0 kind=g key=storage_objects/M_23C00.phys*/
extern int f03d5();
extern int g40c8;
extern char far *rect_queue_write_ptr;
extern char display_mode;
extern int gc0bc;
extern void board_run_unit_script();

/* ---- F_5A3B (original code at 0x5A3B) ---- */
/* F_5A3B -- arm the 0x18-slot cursor/marker trail at the current hot spot,
   but only when that spot is inside the play window.  The four-term guard
   is an OR of the four out-of-window tests with a plain `return`, so the
   last term is inverted and jumps INTO the body while the fall-through is
   the return's jump to the epilogue. */
void cursor_trail_arm()
{
    int i, x, y;

    gc04e = 0x17;
    x = g736 + 0x10;
    y = g738 + 4;
    if (x < 8 || x > 0x137 || y < 0x10 || y > 0x9f)
        return;
    for (i = 0; i < 0x18; i++) {
        gc050[i] = x;
        gc080[i] = y;
    }
    if (g73a)
        gc0b8 = 9;
    else
        gc0b8 = 3;
    gc0b0 = 0;
    gc0c0 = 0x18;
    g8fe = 1;
    gc0b6 = 0;
}


/* ---- F_5AC3 (original code at 0x5AC3) ---- */
/* F_5AC3 -- the beam walk: step the ray eight times through the tile grid,
   test each step against the shadow bitmap and the object list, then rebuild
   the dirty rectangle from the 24 recorded points. */
void board_raycast_step()
{
    int savemode;                       /* bp-1A */
    int i;                              /* bp-18 */
    int fx;                             /* bp-16 */
    int fy;                             /* bp-14 */
    int dx;                             /* bp-12 */
    int dy;                             /* bp-10 */
    int cell;                           /* bp-0E */
    int recheck;                        /* bp-0C */
    int obj;                            /* bp-0A */
    int hit;                            /* bp-08 */
    int live;                           /* bp-06 */
    struct B3 b;                        /* bp-04 .. bp-02 */
    register int x, y;                  /* si, di */

    recheck = 1;
    live = 0;
    savemode = f020f();
    gc0be = 0;
    x = tx[gc04e];
    y = ty[gc04e];
    fx = x & 7;
    fy = y & 7;
    cell = ((y >> 3) - 2) * 0x26 + (x >> 3) - 1;
    for (i = 0; i < 8; i++) {
        if (x != 0) {
            dx = ((int far *)((char far *)wig + gc0b8 * 0x18))[gc0b0];
            gc0b0++;
            dy = ((int far *)((char far *)wig + gc0b8 * 0x18))[gc0b0];
            gc0b0++;
            if (gc0b0 >= 12) gc0b0 = 0;
            x += dx;
            y += dy;
            fx += dx;
            fy += dy;
            if (fx & 8) {
                if (fx < 0) cell--;
                else cell++;
                fx &= 7;
                recheck = 1;
            }
            if (fy & 8) {
                if (fy < 0) cell -= 0x26;
                else cell += 0x26;
                fy &= 7;
                recheck = 1;
            }
            if (x < 8 || x > 0x137 || y < 0x10 || y > 0x9f) {
                x = 0;
            } else if (recheck != 0) {
                if (vram[cell] & 7) x = 0;
                recheck = 0;
            }
            if ((i & 3) == 0 && x != 0) {
                if ((obj = rect_table_hit_id(x >> 1, y, 1, 1)) >= 8 && obj <= 0x1f) {
                    if ((gc0ba = record_field_skip_n(obj - 8))[1] == 2) {
                        gc0be = 1;
                        x = 0;
                        live = gc0b6 = 0;
                        goto after;
                    }
                }
                if (obj >= 0x30 && obj <= 0x4f) {
                    live = 1;
                    b = *(struct B3 far *)(objtab + (obj - 0x30) * 3 + 1);
                    gc0b2 = b.a << 1;
                    gc0b4 = b.b;
                    gc0c2 = b.c & 0x1f;
                    gc0c4 = sprbase + gc0c2 * 0x1e6 + 0x24;
                } else {
                    live = gc0b6 = 0;
                }
            }
after:
            if (live != 0 && gc0b6 == 0) {
                if ((hit = board_raycast_hit_test(x, y)) == 1) {
                    gc0b8 = gc0c2 - gc0b8;
                    gc0b6 = 1;
                    fcaf1(15);
                } else if (hit == 2) {
                    gc0b8 = gc0c2 - gc0b8 - 8;
                    gc0b6 = 1;
                    fcaf1(15);
                } else if (hit == 3) {
                    gc0b8 = gc0c2 - gc0b8 + 8;
                    gc0b6 = 1;
                    fcaf1(15);
                }
                if (gc0b6 != 0) {
                    while (gc0b8 < 0) gc0b8 += 12;
                    while (gc0b8 >= 12) gc0b8 -= 12;
                }
            }
        } else {
            x = 0;
            if (--gc0c0 <= 0) g8fe = 0;
        }
        if (++gc04e >= 0x18) gc04e = 0;
        tx[gc04e] = x;
        ty[gc04e] = y;
    }
    gc046 = 0x140;
    gc04a = 0xc8;
    gc048 = 0;
    gc04c = 0;
    gfx_color_select(14);
    for (i = 0; i < 0x18; i++) {
        if ((x = tx[i]) != 0) {
            f03d2(x, y = ty[i]);
            if (x < gc046) gc046 = x;
            if (gc048 < x) gc048 = x;
            if (y < gc04a) gc04a = y;
            if (gc04c < y) gc04c = y;
        }
    }
    if (gc048 != 0) {
        *edge = gc046 >> 1;
        edge++;
        *edge = gc04a;
        edge++;
        *edge = (gc048 - gc046 + 4) >> 1;
        edge++;
        *edge = (char)gc04c - (char)gc04a + 1;
        edge++;
    }
    gfx_color_select(savemode);
}


/* ---- F_5E98 (original code at 0x5E98) ---- */
/* F_5E98 -- redraw the 0x18 sprite slots, then append four bytes to the
   command stream at rect_queue_write_ptr.  Plain C. */
void sprite_slots_redraw(void)
{
    int w4, saved;
    register int i, p;

    saved = f020f();
    for (i = 0; i < 0x18; i++) {
        if ((p = gc050[i]) != 0) {
            g40c8 = f03d5(p, (w4 = gc080[i]) + 0xb8);
            f03d2(p, w4);
        }
    }
    if (gc048 != 0) {
        *rect_queue_write_ptr++ = gc046 >> 1;
        *rect_queue_write_ptr++ = *(char *)&gc04a;
        *rect_queue_write_ptr++ = (gc048 - gc046 + 4) >> 1;
        *rect_queue_write_ptr++ = *(char *)&gc04c - *(char *)&gc04a + 1;
    }
    gfx_color_select(saved);
}


/* ---- F_5F3C (original code at 0x5F3C) ---- */
board_raycast_hit_test(x,y) register int x,y;{char c;if(display_mode==5){switch(f03d5(x,y+0xb8)){case 0x83:return 1;case 0x86:return 2;case 0x8a:return 3;default:return 0;}}else{x-=gc0b2;y-=gc0b4;c=gc0c4[(y<<4)-y+(x>>1)];if(x&1)c&=15;else c=(c>>4)&15;if(display_mode==2){switch(c){case 2:return 1;case 3:return 2;case 4:return 3;default:return 0;}}else{switch(c){case 11:return 1;case 9:return 2;case 8:return 3;default:return 0;}}}}


/* ---- F_6021 (original code at 0x6021) ---- */
/* F_6021 -- frameless (no parameters, no locals). */
void board_unit_script_trigger(void)
{
    if (gc0be)
        board_run_unit_script((int)gc0ba, gc0bc);
}
