/* F_A658 -- a menu loop: init calls, a sparse 4-case switch on a key code
   dispatched through a cs: table, and teardown. */
extern int  f6b74(void);          /* 6B74 */
/*@SYM _f6b74=0x6B74 kind=f key=functions/F_6B74.entry*/
extern void f6990(void);          /* 6990 */
/*@SYM _f6990=0x6990 kind=f key=functions/F_6990.entry*/
extern void f6b66(void);          /* 6B66 */
/*@SYM _f6b66=0x6B66 kind=f key=functions/F_6B66.entry*/
extern void f703e(void);          /* 703E */
/*@SYM _f703e=0x703E kind=f key=functions/F_703E.entry*/
extern void fd593(void);          /* D593 */
/*@SYM _fd593=0xD593 kind=f key=functions/F_D593.entry*/
extern int  f792c(void);          /* 792C */
/*@SYM _f792c=0x792C kind=f key=functions/F_792C.entry*/
extern void f7925(void);          /* 7925 */
/*@SYM _f7925=0x7925 kind=f key=functions/F_7925.entry*/
extern void box(int a, int b, int c, int d);        /* 039F */
/*@SYM _box=0x039F kind=f key=functions/F_039F.entry*/
extern void f8480(char far *s, int a);               /* 8480 */
/*@SYM _f8480=0x8480 kind=f key=functions/F_8480.entry*/
extern void f01ce(int a);                            /* 01CE */
/*@SYM _f01ce=0x01CE kind=f key=functions/F_01CE.entry*/
extern void bar(int a, int b, int c);               /* 03A2 */
/*@SYM _bar=0x03A2 kind=f key=functions/F_03A2.entry*/
extern void fill(int a, int b, int c, int d);       /* 03AB */
/*@SYM _fill=0x03AB kind=f key=functions/F_03AB.entry*/
extern int  faf45(void);                           /* AF45 */
/*@SYM _faf45=0xAF45 kind=f key=functions/F_AF45.entry*/
extern void fa19d(void);                              /* A19D */
/*@SYM _fa19d=0xA19D kind=f key=functions/F_A19D.entry*/
extern void f8453(void);          /* 8453 */
/*@SYM _f8453=0x8453 kind=f key=functions/F_8453.entry*/
extern void f791e(void);          /* 791E */
/*@SYM _f791e=0x791E kind=f key=functions/F_791E.entry*/
extern void fd5a6(void);          /* D5A6 */
/*@SYM _fd5a6=0xD5A6 kind=f key=functions/F_D5A6.entry*/
extern void f7162(void);          /* 7162 */
/*@SYM _f7162=0x7162 kind=f key=functions/F_7162.entry*/
extern void f6997(void);          /* 6997 */
/*@SYM _f6997=0x6997 kind=f key=functions/F_6997.entry*/
extern void f6b66(void);          /* 6B66 */
/*@SYM _f6b66=0x6B66 kind=f key=functions/F_6B66.entry*/

int fa658(void)
{
    register int sel, quit;
    int sv2, sv1;

    sel = 0x10;
    quit = 0;
    sv2 = f6b74();
    f6990(); f6b66(); f703e(); fd593();
    sv1 = f792c();
    f7925();
    box(0, 0, 0x140, 0xc8);
    f8480("", 1);
    f01ce(0);
    bar(0x26, 0x73, 0xf4);
    box(0x24, 0x73, 0xf6, 1);
    fill(0x28, 0x7e, 0xee, 0xa);
    box(0x28, 0x7e, 0xee, 0x14);
    while (!quit) {
        switch (faf45()) {
        case 0x1b:  quit = 1; sel = 0;     break;
        case 0x148:
        case 0x150: sel ^= 0x30; fa19d();    break;
        case 0x0d:  quit = sel;            break;
        }
    }
    f8453();
    if (sv1) f791e();
    fd5a6(); f7162();
    if (!sv2) f6997();
    f6b66();
    return sel;
}
