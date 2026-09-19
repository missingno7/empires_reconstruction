struct CEL { char b[0x319]; };
extern struct CEL cel[];
extern int  xa[], ya[];         /* DS:8BEA, DS:8BF4 */
/*@SYM _xa=0x8BEA kind=g key=storage_objects/M_2871A.phys*/
/*@SYM _ya=0x8BF4 kind=g key=storage_objects/M_28724.phys*/
extern int  g72c, g72e, g736, g738, g73a, g96, gbc;
extern char far *pool;                  /* DS:99D2 */
/*@SYM _pool=0x99D2 kind=g key=storage_objects/M_29502.phys*/
extern char far *src, far *dst;         /* DS:C5CA -> DS:40C4 */
/*@SYM _src=0xC5CA kind=g key=storage_objects/M_2C0FA.phys*/
/*@SYM _dst=0x40C4 kind=g key=storage_objects/M_23BF4.phys*/
extern void fcb48(void);                    /* CB48 */
/*@SYM _fcb48=0xCB48 kind=f key=functions/F_CB48.entry*/
extern void fcaf1(int n);                   /* CAF1 */
/*@SYM _fcaf1=0xCAF1 kind=f key=functions/F_CAF1.entry*/
extern void f22b1(void);                    /* 22B1 */
/*@SYM _f22b1=0x22B1 kind=f key=functions/F_22B1.entry*/
extern void f6c57(int n);               /* 6C57 */
/*@SYM _f6c57=0x6C57 kind=f key=functions/F_6C57.entry*/
extern void blit(int x, int y, char far *s);                    /* 03C9 */
/*@SYM _blit=0x03C9 kind=f key=functions/F_03C9.entry*/
extern void wipe(int x, int y, int w, int h, int x2, int y2);   /* 03B4 */
/*@SYM _wipe=0x03B4 kind=f key=functions/F_03B4.entry*/
extern void copy(int x, int y, char far *s, int n);             /* 03CC */
/*@SYM _copy=0x03CC kind=f key=functions/F_03CC.entry*/
extern void f1ecd(void);                    /* 1ECD */
/*@SYM _f1ecd=0x1ECD kind=f key=functions/F_1ECD.entry*/
extern void f6c6f(void);                    /* 6C6F */
/*@SYM _f6c6f=0x6C6F kind=f key=functions/F_6C6F.entry*/
extern void box(int x, int y, int w, int h);                    /* 039F */
/*@SYM _box=0x039F kind=f key=functions/F_039F.entry*/

void f233e(register int i)
{
    register int k;

    fcb48();
    g72c = 0;
    fcaf1(0x0d);
    g96 = 0x190;
    f22b1();
    gbc = 1;
    for (k = 1; k < 5; k++) {
        f6c57(0x18);
        dst = src;
        blit(xa[i], ya[i] + 0xb8, (char far *)&cel[k]);
        wipe(xa[i], ya[i] + 0xb8, 0x2e, 0x21, xa[i], ya[i]);
        copy(g736, g738, pool + g72e * 0x2a2, g73a);
        f1ecd();
        f6c6f();
    }
    f22b1();
    for (k = 12; k < 16; k++) {
        f6c57(0x18);
        dst = src;
        wipe(xa[i], ya[i] + 0xb8, 0x2e, 0x28, xa[i], ya[i]);
        f22b1();
        copy(xa[i] + 4, ya[i], pool + k * 0x2a2, g73a);
        f1ecd();
        f6c6f();
    }
    gbc = 0;
    fcaf1(0x0d);
    for (k = 3; k >= 0; k--) {
        f6c57(0x18);
        blit(xa[i], ya[i], (char far *)&cel[k]);
        box(xa[i], ya[i], 0x2e, 0x21);
        f6c6f();
    }
    g96 = 0x9f;
}
