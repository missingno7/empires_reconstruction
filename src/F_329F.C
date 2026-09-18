struct REC3E8 { char b[0x3e8]; };
extern struct REC3E8 g43b4[];
extern int gbfba;
extern char far *gbfbc;
extern char far *src, far *dst;               /* DS:C5CA -> DS:40C4 */
/*@SYM _src=0xC5CA kind=g key=storage_objects/M_2C0FA.phys*/
/*@SYM _dst=0x40C4 kind=g key=storage_objects/M_23BF4.phys*/
extern int gbc, g40ce;
extern void f2ae2(void);
extern void wipe(int x, int y, int w, int h, int x2, int y2);   /* 03B4 */
/*@SYM _wipe=0x03B4 kind=f key=functions/F_03B4.entry*/
extern void f4eeb(int n);

void f329f(void)
{
    gbfbc = (char far *) &g43b4[gbfba];
    f2ae2();
    dst = src;
    wipe(8, 0xc8, 0x130, 0x90, 8, 0x10);
    g40ce = gbc = 0;
    f4eeb(0);
    gbc = 1;
}
