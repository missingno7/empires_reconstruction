/* F_462E -- paint the title/menu backdrop for the current mode.  si is the
   mode, di the row offset chosen by the two-term disjunction at 4698. */
extern int g9ade;
extern int board_record_index;
extern char far *board_records;
extern char far *ui_gfx_shadow_a;
extern unsigned char g4374[];
extern char g0b3ae[];
extern int f1d47(), value_parity(), resource_load_record(), face7();extern void setmem();extern void movmem();
extern void f4517();

#include "R3E8.H"
extern struct record3e8 g43b4[];

void menu_backdrop_paint(void)
{
    register int m, d;

    f1d47();
    if (value_parity(g9ade))
        m = 0x14;
    else
        m = g9ade / 2;
    if (m == 0x15)
        resource_load_record(0x42);
    else
        resource_load_record(m + 0x1000);
    if (m == 0x14) {
        movmem(ui_gfx_shadow_a, g4374, 0x2750);
        setmem(g0b3ae, 0xbb8, 0);
    } else {
        if (face7() == 0 || m > 0x14)
            d = 0;
        else
            d = 0x330c;
        movmem(ui_gfx_shadow_a + d + 2, g4374, 0x2750);
        movmem(ui_gfx_shadow_a + d + 0x2754, g0b3ae, 0xbb8);
    }
    if (m < 0x14)
        f4517(g4374[0] & 0x7f);
    board_records = (char far *) &g43b4[board_record_index = 0];
}