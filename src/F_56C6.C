/* F_56C6 -- the intro chapter's driver.  Entry 156C6, 885 bytes.

   THE TWO `msg` SITES PUSH ONE ADDRESS, NOT TWO.  At extent offsets 243 and
   435 the image holds the same six bytes -- `1E` push ds / `B8 9D 13` mov
   ax,0x139D / `50` push ax -- so both calls hand `msg` the far pointer
   DS:0x139D, one object referenced twice.  An earlier draft wrote the string
   literal "a" at both sites; TC 2.0 does not merge identical string
   literals, so that spelling put TWO two-byte objects in this module's
   `_DATA` at addends 0 and 2, and the provider's module-data-base vote
   (pf/82 section 5c) then read two bases 0x139B and 0x139D from the two
   sites and refused REFUSE_MODULE_DATA_BASE_DISAGREES -- the draft's own two
   sites did not corroborate each other.  ONE object referenced twice gives
   two fixups with ONE addend, the two votes agree, and the base is decided.

   WHAT IS ACTUALLY AT DS:0x139D, read off the image (DGROUP is image address
   0xFA30): the two bytes `07 00`, inside a 20-byte record table whose
   records start at DGROUP 0x1377 and repeat every 20 bytes (0x1377, 0x138B,
   0x139F, 0x13B3, 0x13C7 ...), each carrying two DS-relative far pointers
   into the sign-in strings that begin at 0x1340 and 0x13F1.  So the object
   is NOT a caption, and this extent establishes nothing about its type or
   its owner beyond the address and those two bytes.  The draft therefore
   declares the smallest module-local object the bytes support -- an
   initialised two-byte char array holding exactly `07 00` -- and passes it
   by name, which is the array-name push shape (`push ds` then `mov
   ax,offset`, tc20-codegen rule 5) the image carries.  It is not a claim
   that the object is this module's; it is the one statement in C that
   reproduces both the bytes and the single addend the two sites share. */

extern int  g857, g94, g96, g98, g9a, g1770, g1774, g1776;
extern unsigned char vmode;              /* DS:BFCD */
/*@SYM _vmode=0xBFCD kind=g key=storage_objects/M_2BAFD.phys*/
extern char far *t1, far *t2, far *t3, far *t4;   /* BFE2, BFE6, BFDE, BFF2 */
/*@SYM _t1=0xBFE2 kind=g key=storage_objects/M_2BB12.phys*/
/*@SYM _t2=0xBFE6 kind=g key=storage_objects/M_2BB16.phys*/
/*@SYM _t3=0xBFDE kind=g key=storage_objects/M_2BB0E.phys*/
/*@SYM _t4=0xBFF2 kind=g key=storage_objects/M_2BB22.phys*/
extern char buf[];                       /* DS:BFEE */
/*@SYM _buf=0xBFEE kind=g key=storage_objects/REGION_2BB1E.phys*/
extern void f01ce(int n);                 /* 01CE */
/*@SYM _f01ce=0x01CE kind=f key=functions/F_01CE.entry*/
extern void clear(int a,int b,int c,int d);   /* 03A8 */
/*@SYM _clear=0x03A8 kind=f key=functions/F_03A8.entry*/
extern void box(int a,int b,int c,int d);     /* 039F */
/*@SYM _box=0x039F kind=f key=functions/F_039F.entry*/
extern void fd5ba(int n);                /* D5BA */
extern void f555b(void);                 /* 555B */
extern void f684a(int n, char far * far *p);   /* 684A */
/*@SYM _f684a=0x684A kind=f key=functions/F_684A.entry*/
extern void f55c7(void);                 /* 55C7 */
extern void copy(int a,int b,char far *s,int n);   /* 03CC */
/*@SYM _copy=0x03CC kind=f key=functions/F_03CC.entry*/
extern void f5673(void);                 /* 5673 */
extern void fd5f9(int n);                /* D5F9 */
extern void f5321(char far *s,int a,int b,char far *d);  /* 5321 */
/*@SYM _f5321=0x5321 kind=f key=functions/F_5321.entry*/
extern void f75f3(void);                 /* 75F3 */
extern void f6c57(int n);                /* 6C57 */
/*@SYM _f6c57=0x6C57 kind=f key=functions/F_6C57.entry*/
extern int  f5593(void);                /* 5593 */
/*@SYM _f5593=0x5593 kind=f key=functions/F_5593.entry*/
extern int  f86c9(char far *s);            /* 86C9 */
/*@SYM _f86c9=0x86C9 kind=f key=functions/F_86C9.entry*/
extern void f9f40(int a,int b,int c,int d,int e,int f); /* 9F40 */
/*@SYM _f9f40=0x9F40 kind=f key=functions/F_9F40.entry*/
extern void f6d3c(int a,int b,char far *s);   /* 6D3C */
extern void f568c(void);                 /* 568C */
extern void fcaf1(int n);                /* CAF1 */
extern void fcb48(void);                 /* CB48 */
extern void f6c26(int n);                /* 6C26 */
extern void farfree(char far *p);            /* F6C3 */
/*@SYM _farfree=0xF6C3 kind=f key=functions/F_F6C3.entry*/

/* DS:0x139D -- see the head comment; both `msg` sites push this one address.
   The `g<hex>`/`f<hex>` convention does not cover this name, so the address
   is DECLARED (ae/59 section 8) and it comes from the MODEL object the key
   names, never from the image at 0x139D.  THE CENSUS NAMES ONE THERE:
   G_P20ECD, a PROVEN_DYNAMIC two-byte scalar at exactly DGROUP+0x139D with
   eleven read sites and no writer -- and one of the eleven is 18815, inside
   F_86C9, which is `msg`, the callee both sites hand this address to.  So
   the object the draft declares and the object the program reads are the
   same word, corroborated from the other end. */
/*@SYM _q139d=0x139D kind=g key=storage_objects/G_P20ECD.phys*/
static char q139d[2] = { 7, 0 };

int f56c6(void)
{
    int key;
    register int again, k;

    again = 1;
    if (g857 == 0) {
        g96 = 0x18f; g98 = 0; g9a = 0xa0;
        f01ce(0);
        clear(0, 0, 0x140, 0xc8);
        fd5ba(0x35);
        f555b();
        f684a(0x38, &t1);
        f684a(0x37, &t2);
        f55c7();
        copy(0xec, 0x89, t4, 0);
        f5673();
        box(0, 0, 0x140, 0xc8);
        g1776 = 1;
        fd5f9(0x35);
        f5321(t1 + 2, 0x34, 0x1e, buf);
        f75f3();
top:
        while (again) {
                f6c57(0x1bc6);
                key = f5593();
                switch (key) {
                case 0x1b: if (f86c9(q139d) == 1) return -1; break;
                case -1:
                case 0x0d: again = 0;
                default:   break;
                }
            }
            again = 1;
            for (k = 0; k < 2; k++) {
                f9f40(0, 0xe8, 0xc8, 0x8a, 0, 0x20);
                if (vmode == 2) f01ce(5); else f01ce(0xf);
                f6d3c(8, 0x1e, t2 + ((int far *)t2)[k] + 2);
                f75f3();
                box(0, 0, 0x140, 0xc8);
                while (again) {
                    f6c57(0x1bc6);
                    key = f5593();
                    switch (key) {
                    case 0x1b: if (f86c9(q139d) == 1) return -1; break;
                    case -1:
                    case 0x0d: again = 0;
                default:   break;
                    }
                }
                again = 1;
            }
        if (k == 2 && key == -1) { f568c(); goto top; }
        f9f40(0, 0x17e, 0xbc, 0x10, 0, 0xb6);
        g94 = 0; g9a = 0x9f; g1774 = 1;
        fcaf1(0x18);
        f5321(t1 + 0x344, 0xa, 0x2d, buf);
        g1770 = 1;
        fcb48();
        g1774 = 0;
        f6c26(0xed);
        f5321(t1 + 0x498, 0x16, 0x1e, buf);
        f5321(t1 + 0x3e6, 0xb, 0x28, buf);
        f6c26(0xed);
        g1774 = 1;
        fcaf1(0x18);
        f5321(t1 + 0x66c, 0xb, 0x28, buf);
        g1774 = 0;
        g1770 = 1;
        fcb48();
        f5321(t1 + 0x5fa, 7, 0x1e, buf);
        g94 = 0x10; g96 = 0x9f; g98 = 4; g9a = 0x9b;
        farfree(t1); farfree(t2); farfree(t3);
    } else {
        f01ce(0);
        clear(0, 0, 0x140, 0xc8);
        box(0, 0, 0x140, 0xc8);
    }
    return 1;
}
