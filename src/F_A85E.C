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
extern void f86c9(char far *s);                       /* 86C9 */
/*@SYM _f86c9=0x86C9 kind=f key=functions/F_86C9.entry*/
extern int  f6b74(void);                           /* 6B74 */
/*@SYM _f6b74=0x6B74 kind=f key=functions/F_6B74.entry*/
extern void f6990(void), f6b66(void), f703e(void), fd593(void);
extern int  f792c(void);                           /* 792C */
/*@SYM _f792c=0x792C kind=f key=functions/F_792C.entry*/
extern void f7925(void);                           /* 7925 */
/*@SYM _f7925=0x7925 kind=f key=functions/F_7925.entry*/
extern void f8480(char far *s, int a);               /* 8480 */
/*@SYM _f8480=0x8480 kind=f key=functions/F_8480.entry*/
extern void f01ce(int a);                            /* 01CE */
/*@SYM _f01ce=0x01CE kind=f key=functions/F_01CE.entry*/
extern void f0355(int a, int b, int c, int d);      /* 0355 */
/*@SYM _f0355=0x0355 kind=f key=functions/F_0355.entry*/
extern void box(int a, int b, int c, int d);        /* 039F */
/*@SYM _box=0x039F kind=f key=functions/F_039F.entry*/
extern int  fa525(void);                             /* A525 */
/*@SYM _fa525=0xA525 kind=f key=functions/F_A525.entry*/
extern void f8453(void), f791e(void), fd5a6(void), f7162(void);
extern void f6997(void), f6b66(void);
extern void setmem(struct S far *p, unsigned n, int v); /* F304 */
/*@SYM _setmem=0xF304 kind=f key=functions/F_F304.entry*/
extern int  fa658(void);                            /* A658 */
extern void strcpy(struct S far *p, char far *s);      /* F2DB */
/*@SYM _strcpy=0xF2DB kind=f key=functions/F_F2DB.entry*/

int fa85e(void)
{
    register int rc, flag;
    int sv;

    if (cur == 10) { f86c9(g1389); err = 2; return -2; }
    flag = f6b74();
    f6990(); f6b66(); f703e(); fd593();
    sv = f792c();
    f7925();
    f8480(g1375, 1);
    f01ce(0);
    f0355(0x58, 0x5f, 0x78, 0x13);
    box(0x58, 0x5f, 0x78, 0x13);
    rc = fa525();
    f8453();
    if (sv) f791e();
    fd5a6(); f7162();
    if (!flag) f6997();
    f6b66();
    if (rc > -1) {
        rc = cur;
        setmem((struct S far *)tbl + rc, 0x1b, 0);
        if ((tbl[rc].b11 = fa658()) != 0) {
            tbl[rc].l21 = 4;
            tbl[rc].a9  = 1;
            tbl[rc].d13 = g176e;
            tbl[rc].f15 = g1772;
            if (tbl[rc].b11 == 0x10) tbl[rc].h17 = 1;
            tbl[rc].j19 = -1;
            strcpy(&tbl[rc], g12d0);
        } else {
            err = 1; rc = -2;
        }
    } else if (cur != 0) { err = 1; rc = -2; }
    return rc;
}

