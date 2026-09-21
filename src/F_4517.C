extern int resource_load_record();
extern void movmem();
extern void resource_load_record_into();
extern int g73c;
extern int g724[];
extern char far *ui_gfx_shadow_a;
extern char s79bf[];
extern char a74a2[];
extern char s8c12[];
extern char far *g99d6;
extern char far *a72b2[];

void f4517(di)
int di;
{
    char far *d;
    register int i;

    if (g73c == di)
        return;
    g73c = di;
    resource_load_record(di + 0x1015);
    d = ui_gfx_shadow_a + 2;
    for (i = 0; i < 5; i++, d += 0x31b)
        movmem(d, s79bf + i * 0x319, 0x319);
    for (i = 0; i < 6; i++, d += 0xbd)
        movmem(d, a74a2 + i * 0xbb, 0xbb);
    movmem(d, s8c12, 0xad2);
    resource_load_record_into(di + 0x1019, g99d6);
    i = 0;
    for (; i < g724[g73c]; i++)
        a72b2[i] = g99d6 + ((unsigned far *) g99d6)[i] + 2;
    for (; i < 0x28; i++)
        a72b2[i] = a72b2[0];
}
