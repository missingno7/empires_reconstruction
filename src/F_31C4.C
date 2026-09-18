/* F_31C4 -- scroll the board into view, redraw the frame, scroll it out.
   si is the row cursor in both loops; gc5ca is copied into g40c4 as a far
   pointer (les bx / mov es / mov bx). */
extern int g072e, g00bc;
extern char far *gc5ca, *g40c4;
extern int f6c57(), f22b1(), f2269(), f1ecd(), f6c6f(), f6c26();
extern int fcb48(), f8bab(), f2ae2(), f03b4(), f4eeb(), f039f(), f6fda();

void f31c4(void)
{
    register int y;

    if (g072e <= 8) {
        g40c4 = gc5ca;
        for (y = 0x10; y < 0x14; y++) {
            f6c57(0x30);
            g072e = y;
            f22b1();
            f2269();
            f1ecd();
            f6c6f();
        }
        f6c26(0x30);
    }
    g00bc = 0;
    fcb48();
    f8bab();
    f2ae2();
    f03b4(8, 0xc8, 0x130, 0x90, 8, 0x10);
    f4eeb(0);
    f2269();
    f039f(8, 0x10, 0x130, 0x90);
    f6fda();
    g00bc = 1;
    g40c4 = gc5ca;
    if (g072e == 0x13) {
        for (y = 0x13; y >= 0x10; y--) {
            f6c57(0x30);
            g072e = y;
            f22b1();
            f2269();
            f1ecd();
            f6c6f();
        }
        g072e = 0;
        f6c26(0x30);
    }
}
