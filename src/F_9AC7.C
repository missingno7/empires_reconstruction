extern void fcaf1(int n);
extern void wipe(int x, int y, int w, int h, int x2, int y2);   /* 03B4 */
/*@SYM _wipe=0x03B4 kind=f key=functions/F_03B4.entry*/
extern void box(int x, int y, int w, int h);                    /* 039F */
/*@SYM _box=0x039F kind=f key=functions/F_039F.entry*/
extern void f6c26(int n);
extern void fcb48(void);
extern void fd4b3(int n);

void f9ac7(void)
{
    int i;

    for (i = 0; i < 3; i++) {
        fcaf1(0x15);
        wipe(0xf4, i * 0x30 + 0x158, 0x30, 0x23, 0xf4, i * 0x30 + 0x10);
        box(0xf4, i * 0x30 + 0x10, 0x30, 0x23);
        wipe(0xf4, i * 0x30 + 0x158, 0x30, 0x23, 0xf4, i * 0x30 + 0xc8);
        f6c26(0x78);
        fcb48();
    }
    fd4b3(5);
}
