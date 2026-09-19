/* F_A658 -- a menu loop: init calls, a sparse 4-case switch on a key code
   dispatched through a cs: table, and teardown. */
extern int  init_a(void);          /* 6B74 */
/*@SYM _init_a=0x6B74 kind=f key=functions/F_6B74.entry*/
extern void init_b(void);          /* 6990 */
/*@SYM _init_b=0x6990 kind=f key=functions/F_6990.entry*/
extern void init_c(void);          /* 6B66 */
/*@SYM _init_c=0x6B66 kind=f key=functions/F_6B66.entry*/
extern void init_d(void);          /* 703E */
/*@SYM _init_d=0x703E kind=f key=functions/F_703E.entry*/
extern void init_e(void);          /* D593 */
/*@SYM _init_e=0xD593 kind=f key=functions/F_D593.entry*/
extern int  init_f(void);          /* 792C */
/*@SYM _init_f=0x792C kind=f key=functions/F_792C.entry*/
extern void init_g(void);          /* 7925 */
/*@SYM _init_g=0x7925 kind=f key=functions/F_7925.entry*/
extern void box(int a, int b, int c, int d);        /* 039F */
/*@SYM _box=0x039F kind=f key=functions/F_039F.entry*/
extern void text(char far *s, int a);               /* 8480 */
/*@SYM _text=0x8480 kind=f key=functions/F_8480.entry*/
extern void f01ce(int a);                            /* 01CE */
/*@SYM _f01ce=0x01CE kind=f key=functions/F_01CE.entry*/
extern void bar(int a, int b, int c);               /* 03A2 */
/*@SYM _bar=0x03A2 kind=f key=functions/F_03A2.entry*/
extern void fill(int a, int b, int c, int d);       /* 03AB */
/*@SYM _fill=0x03AB kind=f key=functions/F_03AB.entry*/
extern int  faf45(void);                           /* AF45 */
/*@SYM _faf45=0xAF45 kind=f key=functions/F_AF45.entry*/
extern void act(void);                              /* A19D */
/*@SYM _act=0xA19D kind=f key=functions/F_A19D.entry*/
extern void done_a(void);          /* 8453 */
/*@SYM _done_a=0x8453 kind=f key=functions/F_8453.entry*/
extern void done_b(void);          /* 791E */
/*@SYM _done_b=0x791E kind=f key=functions/F_791E.entry*/
extern void done_c(void);          /* D5A6 */
/*@SYM _done_c=0xD5A6 kind=f key=functions/F_D5A6.entry*/
extern void done_d(void);          /* 7162 */
/*@SYM _done_d=0x7162 kind=f key=functions/F_7162.entry*/
extern void done_e(void);          /* 6997 */
/*@SYM _done_e=0x6997 kind=f key=functions/F_6997.entry*/
extern void done_f(void);          /* 6B66 */
/*@SYM _done_f=0x6B66 kind=f key=functions/F_6B66.entry*/

int fa658(void)
{
    register int sel, quit;
    int sv2, sv1;

    sel = 0x10;
    quit = 0;
    sv2 = init_a();
    init_b(); init_c(); init_d(); init_e();
    sv1 = init_f();
    init_g();
    box(0, 0, 0x140, 0xc8);
    text("", 1);
    f01ce(0);
    bar(0x26, 0x73, 0xf4);
    box(0x24, 0x73, 0xf6, 1);
    fill(0x28, 0x7e, 0xee, 0xa);
    box(0x28, 0x7e, 0xee, 0x14);
    while (!quit) {
        switch (faf45()) {
        case 0x1b:  quit = 1; sel = 0;     break;
        case 0x148:
        case 0x150: sel ^= 0x30; act();    break;
        case 0x0d:  quit = sel;            break;
        }
    }
    done_a();
    if (sv1) done_b();
    done_c(); done_d();
    if (!sv2) done_e();
    done_f();
    return sel;
}
