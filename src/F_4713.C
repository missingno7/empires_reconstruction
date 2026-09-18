/* F_4713 -- the level driver: set the map geometry, build the view, then run
   the turn loop in F_3A75 and hand back its result.  No frame at all (no
   parameters, no locals, one register variable), which is what -k- gives. */
extern int f462e(), f7df1(), f1ec0(), f2ae2(), f9b68(), f8a37(), f726a();
extern int f01ce(), f03a8(), f9f40(), f4eeb(), f2269(), f6fda(), f039f();
extern int f1eb4(), fd4b3(), fd5f9(), f1ea5(), f3a75(), fc15e();

extern unsigned char b4374, b4375, b4376;
extern int g71e, g722, g72c, g72e, g736, g738, g73a, g73e;
extern int g1776, g8bea, g8bec, g8bee, g8bf4, g8bf6, g8bf8, g9ade, gb07a;

int f4713()
{
    register int r;

    f462e();
    f7df1();
    g736 = b4375;
    g736 <<= 1;
    g736 = ((g736 + 3) >> 2) << 2;
    g738 = b4376;
    if (b4374 & 0x80) g73a = 1;
    else g73a = 0;
    g72c = gb07a = g72e = 0;
    if (f1ec0(g9ade)) {
        g722 = 3;
        g8bea = g8bec = g8bee = 0xf4;
        g8bf4 = 0x12;
        g8bf6 = 0x42;
        g8bf8 = 0x72;
        g71e = 1;
        f2ae2();
        f9b68(g9ade >> 1);
    } else {
        g722 = g71e = 0;
        f8a37();
        f726a();
        f2ae2();
    }
    if (g73e == 0) {
        f01ce(1);
        f03a8(8, 0x10, 0x130, 0x90);
    }
    f9f40(8, 0xc8, 0x130, 0x90, 8, 0x10);
    f4eeb(0);
    f2269();
    f6fda();
    f039f(0, 0, 0x140, 0xc8);
    if (f1eb4() != 4 && (g9ade & 7) == 0) {
        fd4b3(0);
        fd4b3(1);
        if (f1eb4() == 1) fd4b3(2);
        else if (f1eb4() == 2) fd4b3(3);
    }
    g1776 = 1;
    if (f1ec0(g9ade) == 0) {
        if (f1eb4() != 4)
            fd5f9((f1eb4() << 2) + (g9ade & 2) + 0x1073);
        else
            fd5f9((f1ea5() << 2) + 0x1073);
    }
    r = f3a75();
    if (r != 0 && g9ade == 0x27) {
        r = fc15e();
        g9ade = 0x27;
    }
    return r;
}
