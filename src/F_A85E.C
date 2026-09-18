struct S {
    char pad0[9];
    int  a9;        /* +9  */
    char b11;       /* +11 */
    char pad1;
    int  d13;       /* +13 */
    int  f15;       /* +15 */
    int  h17;       /* +17 */
    int  j19;       /* +19 */
    char l21;       /* +21 */
    char pad2[5];
};                  /* sizeof == 27 */

extern struct S tbl[];
extern char g1389[], g1375[], g12d0[];
extern int cur, err, g176e, g1772;
extern void msg(char far *s);                       /* 86C9 */
/*@SYM _msg=0x86C9 kind=f key=functions/F_86C9.entry*/
extern int  init_a(void);                           /* 6B74 */
/*@SYM _init_a=0x6B74 kind=f key=functions/F_6B74.entry*/
extern void init_b(void), init_c(void), init_d(void), init_e(void);
extern int  init_f(void);                           /* 792C */
/*@SYM _init_f=0x792C kind=f key=functions/F_792C.entry*/
extern void init_g(void);                           /* 7925 */
/*@SYM _init_g=0x7925 kind=f key=functions/F_7925.entry*/
extern void text(char far *s, int a);               /* 8480 */
/*@SYM _text=0x8480 kind=f key=functions/F_8480.entry*/
extern void mode(int a);                            /* 01CE */
/*@SYM _mode=0x01CE kind=f key=functions/F_01CE.entry*/
extern void frame(int a, int b, int c, int d);      /* 0355 */
/*@SYM _frame=0x0355 kind=f key=functions/F_0355.entry*/
extern void box(int a, int b, int c, int d);        /* 039F */
/*@SYM _box=0x039F kind=f key=functions/F_039F.entry*/
extern int  pick(void);                             /* A525 */
/*@SYM _pick=0xA525 kind=f key=functions/F_A525.entry*/
extern void done_a(void), done_b(void), done_c(void), done_d(void);
extern void done_e(void), done_f(void);
extern void clr(struct S far *p, unsigned n, int v); /* F304 */
/*@SYM _clr=0xF304 kind=f key=functions/F_F304.entry*/
extern int  fa658(void);                            /* A658 */
extern void put(struct S far *p, char far *s);      /* F2DB */
/*@SYM _put=0xF2DB kind=f key=functions/F_F2DB.entry*/

int fa85e(void)
{
    register int rc, flag;
    int sv;

    if (cur == 10) { msg(g1389); err = 2; return -2; }
    flag = init_a();
    init_b(); init_c(); init_d(); init_e();
    sv = init_f();
    init_g();
    text(g1375, 1);
    mode(0);
    frame(0x58, 0x5f, 0x78, 0x13);
    box(0x58, 0x5f, 0x78, 0x13);
    rc = pick();
    done_a();
    if (sv) done_b();
    done_c(); done_d();
    if (!flag) done_e();
    done_f();
    if (rc > -1) {
        rc = cur;
        clr((struct S far *)tbl + rc, 0x1b, 0);
        if ((tbl[rc].b11 = fa658()) != 0) {
            tbl[rc].l21 = 4;
            tbl[rc].a9  = 1;
            tbl[rc].d13 = g176e;
            tbl[rc].f15 = g1772;
            if (tbl[rc].b11 == 0x10) tbl[rc].h17 = 1;
            tbl[rc].j19 = -1;
            put(&tbl[rc], g12d0);
        } else {
            err = 1; rc = -2;
        }
    } else if (cur != 0) { err = 1; rc = -2; }
    return rc;
}

