/* F_9DCC -- advance the animation one frame, or finish it.  si/di are the
   two register variables holding the blit origin; the char field stores are
   written as the assignment inside the call argument. */
struct anim {
    char pad[0x17];
    char f17;
};
#include "C470.H"

extern struct anim g0dcc[];
extern struct c470_record gc470[];
extern int g13ed;
extern int gc34e, gc350, gc352, gc354, gc359, gc35b, gc35d;
extern char far *gc5ca;
extern void f9d79(), f9d8e(), f039f();
extern int f656c(), f7747(), f7343(), f734e(), f99e2(), f03b4();

int f9dcc(int n)
{
    register int x, y;

    gc35b++;
    if (n + 9 != gc352) {
        f9d79();
        if (gc35b == 1) {
            f656c(g0dcc[gc35d].f17 + 0x1023);
            f7747(gc5ca + 2);
            return 0;
        } else {
            f9d8e();
            f7343(gc470[g13ed].state = 4);
            return -1;
        }
    }
    if (gc35b == 1) {
        f734e(gc470[g13ed].state = 4);
    }
    f99e2(gc359);
    x = gc34e;
    y = gc350;
    f99e2(gc352);
    gc354 = 1;
    f03b4(gc34e, gc350, 0x28, 0x1d, x, y - 0xb8);
    f039f(x, y - 0xb8, 0x28, 0x1d);
    return gc35b;
}
