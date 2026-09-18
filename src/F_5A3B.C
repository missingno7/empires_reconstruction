/* F_5A3B -- arm the 0x18-slot cursor/marker trail at the current hot spot,
   but only when that spot is inside the play window.  The four-term guard
   is an OR of the four out-of-window tests with a plain `return`, so the
   last term is inverted and jumps INTO the body while the fall-through is
   the return's jump to the epilogue. */
extern int g736, g738, g73a, g8fe;
extern int gc04e, gc0b0, gc0b6, gc0b8, gc0c0;
extern int gc050[], gc080[];

void f5a3b()
{
    int i, x, y;

    gc04e = 0x17;
    x = g736 + 0x10;
    y = g738 + 4;
    if (x < 8 || x > 0x137 || y < 0x10 || y > 0x9f)
        return;
    for (i = 0; i < 0x18; i++) {
        gc050[i] = x;
        gc080[i] = y;
    }
    if (g73a)
        gc0b8 = 9;
    else
        gc0b8 = 3;
    gc0b0 = 0;
    gc0c0 = 0x18;
    g8fe = 1;
    gc0b6 = 0;
}
