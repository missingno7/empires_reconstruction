/*@FLAGS -B */
/* F_3A75 -- the turn loop.  Entry 13B75; the declared /24 player-query point
   13C08 is the fd89a() probe at offset 0x93 of this function. */
struct C470 { char pad[0x15]; char b15; char rest[5]; };

extern int f6997(), f6c57(), f6b4a(), f6b1a(), f727d(), fce2a(), fd89a();
extern int fd85f(), f8aa2(), f03b4(), fcaf1(), f2269(), f1ecd(), f31c4();
extern int f734e(), f338a(), f36f0(), f60a9(), f6021(), f233e(), f9dcc();
extern int f22b1(), f4e9f(), f5e98(), f_25b3(), f_d79c(), fd386(), f1f91();
extern int f1f17(), f7277(), f7313(), f5a3b(), f4b0c(), f329f(), f5ac3();
extern int f03cc(), f3986(), f6c6f(), f9ac7();
extern char far *f2a2d();

extern int g0bc, g72c, g72e, g730, g732, g734, g736, g738, g73a;
extern int g8fe, gb68, gb6a, gb6c, gb6e, gb70, g71e, g722, g13ed;
extern int g40ce, g96e4, gb07a, gbfba;
extern char far *g40c4;                 /* the edge cursor */
extern char far *gc5ca;
extern char far *gbfbc;                 /* vram */
extern char far *gbfc0;
extern char far *gbfc4;
extern char far *g40d0;                 /* objtab */
extern char far *g99d2;                 /* sprite base */
extern char str96ee[];
extern unsigned char b740[];
extern unsigned char b4380[], b4386[];
extern char b437a[];
extern char b438c[], b4396[], b43a0[], b43aa[];
extern int w8bea[], w8bf4[];
extern struct C470 c470[];

int f3a75()
{
    int key;                            /* bp-16 */
    int obj;                            /* bp-14 */
    int r;                              /* bp-12 */
    int dir;                            /* bp-10 */
    int oy;                             /* bp-0E */
    int lastobj;                        /* bp-0C */
    int probe;                          /* bp-0A */
    int lastcur;                        /* bp-08 */
    int blink;                          /* bp-06 */
    char far *p;                        /* bp-04 */
    register int x, d;                  /* si, di */

    g730 = g732 = 0;
    x = 0;
    g96e4 = x;
    blink = lastcur = g8fe = lastobj = x;
    g0bc = gb70 = 1;
    f6997();
    for (;;) {
        f6c57(0x18);
        g40ce = dir = key = 0;
        if (f6b4a()) {
            g0bc = 0;
            key = f6b1a();
            if (key == 0xd) f727d();
            else if (key == 0x1b) fce2a();
            g0bc = 1;
        }
        g40c4 = gc5ca;
        if ((obj = fd89a((g736 >> 1) + 1, g738 + 1, 14, 0x27)) != 0 && obj != lastobj) {
            if (obj < 7) {
                fd85f(obj);
                obj--;
                d = b4380[obj];
                d <<= 1;
                oy = b4386[obj];
                b437a[obj] = 0;
                gb07a = f8aa2();
                g40c4 = gc5ca;
                f03b4(d, oy + 0x148, 0x10, 0x10, d, oy + 0xb8);
                f03b4(d, oy + 0xb8, 0x10, 0x10, d, oy);
                fcaf1(2);
                if (gb07a != 0) {
                    f2269();
                    f1ecd();
                    f31c4();
                }
            } else if (obj == 7) {
                f734e(c470[g13ed].b15 = 4);
                fd85f(obj);
                d = ((unsigned char far *)gbfbc)[0x3e5];
                d <<= 1;
                oy = ((unsigned char far *)gbfbc)[0x3e6];
                gbfbc[0x3e7] = 0;
                f03b4(d, oy + 0x148, 0x10, 0x10, d, oy + 0xb8);
                f03b4(d, oy + 0xb8, 0x10, 0x10, d, oy);
                fcaf1(3);
            } else if (obj < 0x20) {
                p = f2a2d(obj - 8);
                if (p[1] != 2) f338a(p);
            } else if (obj < 0x30) {
                f36f0(obj - 0x20);
            }
        }
        lastobj = obj;
        if (*g40d0 != 0) f60a9();
        if (g8fe != 0) f6021();
        if (gb68 != 0 && g722 != 0) {
            for (obj = 0; obj < g722; obj++) {
                if (w8bf4[obj] - 2 <= g738 && w8bf4[obj] + 2 >= g738 &&
                    w8bea[obj] <= g736 && w8bea[obj] + 0x10 >= g736) {
                    if (g71e == 0) {
                        g0bc = 0;
                        f233e(obj);
                        return 1;
                    } else {
                        g0bc = 0;
                        r = f9dcc(obj);
                        if (r != 0) {
                            if (r > 0) {
                                f233e(obj);
                                return 1;
                            }
                            return 0;
                        }
                        g40c4 = gc5ca;
                        g0bc = 1;
                        goto scanned;
                    }
                }
            }
        }
scanned:
        f22b1();
        f4e9f();
        if (g8fe != 0) f5e98();
        f_25b3();
        if (*gbfc0 != 0) f_d79c();
        if (*gbfc4 != 0) fd386();
        if (gb6e != 0) {
            if (x == 0 || gb68 == 0) {
                x = 0;
                g73a = x;
                if ((f1f91(g736 + 0x21, g738 + 1, 0x27) & 7) == 0) {
                    g736 += g734;
                    dir = 1;
                    if (g72e <= 8) {
                        if (++g72e > 8) g72e = 1;
                    }
                }
            }
        } else if (gb6c != 0) {
            if (x == 0 || gb68 == 0) {
                g73a = 1;
                x = 0;
                if ((f1f91(g736, g738 + 1, 0x27) & 7) == 0) {
                    g736 -= g734;
                    dir = -1;
                    if (g72e <= 8) {
                        if (++g72e > 8) g72e = 1;
                    }
                }
            }
        }
        if (gb68 != 0 && (f1f91(g736 + 0x10 - (g73a << 2), g738 + 1, 0x27) & 0x80) != 0) {
                if (x == 0) {
                    if (g736 % 8 == 0) {
                        if ((f1f91(g736 + 0x14, g738 + 1, 0x1d) & 0x80) != 0)
                            g736 += 4;
                        else
                            g736 -= 4;
                    }
                    x = 1;
                } else if ((f1f17(g736 + 0xf, g738 - 4, 2) & 0x80) != 0) {
                    g738 -= 4;
                    if (++x > 2) x = 1;
                } else if ((f1f17(g736 + 0xf, g738 - 2, 2) & 0x80) != 0) {
                    g738 -= 2;
                    if (++x > 2) x = 1;
                }
                g734 = 8;
                g730 = 0;
                g72e = x + 0x13;
        } else if (gb6a != 0 && x != 0) {
            if ((f1f17(g736 + 0xf, g738 + 0x26, 2) & 0x80) != 0) {
                g738 += 4;
                if (++x > 2) x = 1;
                g72e = x + 0x13;
            } else {
                x = 0;
            }
        }
        if (x == 0) {
            if (g730 != 0) {
                if ((f1f17(g736 + 8, g738 - 1, 9) & 7) == 0) {
                    g738 -= b740[g730];
                    g730--;
                    if (++g72e > 0xb) g72e = 0xb;
                    if (dir == 0) g72e = 0xa;
                } else {
                    g730 = 0;
                }
                goto moved;
            }
            if ((f1f17(g736 + 8, g738 + 0x2f, 9) & 7) == 0) {
                if (g732 != 0) {
                    g738 += 8;
                } else {
                    g732 = 1;
                    g738 += 2;
                }
                g72e = (dir & 1) + 0xa;
                goto moved;
            }
            if (((probe = f1f17(g736 + 8, g738 + 0x28, 9)) & 7) == 0) {
                g738 += (((g738 + 0x30) / 8) << 3) - (g738 + 0x28);
                g732 = 0;
                g72e = (dir & 1) + 0xa;
                fcaf1(0xb);
                goto moved;
            }
            if (key == 0x20) {
                if (f7277() == 1) {
                    if ((f1f17(g736 + 8, g738 - 1, 9) & 7) == 0) {
                        fcaf1(0x10);
                        g730 = 8;
                        g72e = 9;
                        g734 = dir ? 8 : 4;
                    } else goto fell;
                } else goto fell;
                goto moved;
            }
            if (gb68 != 0 && gb70 != 0) {
                if ((f1f17(g736 + 8, g738 - 1, 9) & 7) == 0) {
                    gb70 = 0;
                    g730 = 5;
                    g72e = 9;
                    g734 = dir ? 8 : 4;
                    fcaf1(0xc);
                    goto moved;
                }
            }
fell:
            if (dir != 0) {
                if (g72e > 8) g72e = 1;
            } else {
                g72e = 0;
            }
            if (probe & 8) {
                if (probe & 0x10) {
                    if ((f1f91(g736, g738 + 1, 0x27) & 7) == 0) {
                        g736 -= g734;
                        if (g736 <= -4) g736 = -0x11;
                    }
                } else {
                    if ((f1f91(g736 + 0x21, g738 + 1, 0x27) & 7) == 0) {
                        g736 += g734;
                        if (g736 >= 0x130) g736 = 0x131;
                    }
                }
            }
            g734 = 4;
        }
moved:
        if (key == 0x20) {
            if ((obj = f7277()) == 2 && g72c == 0) {
                    if (f7313(-1) != -1) {
                        g72c = 0x3a;
                        blink = 0;
                        fcaf1(0);
                    } else {
                        fcaf1(0x11);
                    }
            } else if (obj == 0 && x == 0) {
                if (g8fe == 0) {
                    f5a3b();
                    fcaf1(0x14);
                } else {
                    fcaf1(0x17);
                }
            }
        }
        if (g72c == 1) g72c = 0;
        f4b0c();
        if (g738 < 0) {
            if (b43a0[gbfba] != 0) {
                g738 = 0x90;
                if (b43a0[gbfba] - 1 != gbfba) {
                    gbfba = b43a0[gbfba] - 1;
                    g8fe = lastcur = lastobj = 0;
                    f329f();
                }
            } else {
                g738 = 0;
            }
        } else if (g738 > 0x90) {
            if (b43aa[gbfba] != 0) {
                g738 = 0;
                if (b43aa[gbfba] - 1 != gbfba) {
                    gbfba = b43aa[gbfba] - 1;
                    g8fe = lastcur = lastobj = 0;
                    f329f();
                }
            } else {
                g738 = 0x90;
            }
        }
        if (g736 < -0x10) {
            if (b438c[gbfba] != 0) {
                g736 = 0x120;
                if (b438c[gbfba] - 1 != gbfba) {
                    gbfba = b438c[gbfba] - 1;
                    g8fe = lastcur = lastobj = 0;
                    f329f();
                }
            } else {
                g736 = -0x10;
            }
        } else if (g736 > 0x130) {
            if (b4396[gbfba] != 0) {
                g736 = 0;
                if (b4396[gbfba] - 1 != gbfba) {
                    gbfba = b4396[gbfba] - 1;
                    g8fe = lastcur = lastobj = 0;
                    f329f();
                }
            } else {
                g736 = 0x130;
            }
        }
        if (g8fe != 0) f5ac3();
        if (blink != 0) {
            blink--;
            if (blink > 0x1a) {
                f03cc(g736, g738, g99d2 + 0x39ec, g73a);
            } else if (blink & 1) {
                f03cc(g736, g738, g99d2 + g72e * 0x2a2, g73a);
            }
        } else if (g72c != 0) {
            f03cc(g736, g738, (char far *)str96ee, 0);
            g72c--;
            f03cc(g736, g738, g99d2 + g72e * 0x2a2, g73a);
        } else {
            if (g40ce != 0 && lastcur != g40ce) {
            f03cc(g736, g738, g99d2 + 0x39ec, g73a);
            blink = 0x1e;
            fcaf1(1);
            f1ecd();
            g0bc = 0;
            if (f734e(-1) == 0) {
                f3986();
                return 0;
            }
            c470[g13ed].b15 = f734e(0);
            g0bc = 1;
            x = 0;
            lastcur = g40ce;
            goto tail;
            } else {
            f03cc(g736, g738, g99d2 + g72e * 0x2a2, g73a);
            }
            lastcur = g40ce;
        }
        f1ecd();
tail:
        f6c6f();
        if (g71e != 0 && g96e4 == 0 && g736 > 0xbe) {
            f9ac7();
            g96e4 = 1;
        }
    }
    g0bc = 0;
    return 1;
}
