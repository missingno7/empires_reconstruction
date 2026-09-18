/* F_747B -- repaint the whole HUD frame: save the current palette/mode word
   f020F returns, switch to 0, lay down the two horizontal rules, the four
   panel fills and the twelve icon blits through the thunks f03A2 / f03A5 /
   f03A8, then restore the saved word. */
extern int f020f(void);
extern void f01ce(int n);
extern void f03a2(int a, int b, int c);
extern void f03a5(int a, int b, int c);
extern void f03a8(int a, int b, int c, int d);

/*@PUB _f747b*/
void f747b(void)
{
    register int save;

    save = f020f();
    f01ce(0);
    f03a5(0, 0xd, 0xba);
    f03a5(0x13f, 0xd, 0xba);
    f03a2(0, 0xc7, 0x140);
    f03a2(6, 0xf, 0x134);
    f03a2(6, 0xa0, 0x134);
    f03a5(6, 0x10, 0x90);
    f03a5(7, 0x10, 0x90);
    f03a5(0x138, 0x10, 0x90);
    f03a5(0x139, 0x10, 0x90);
    f03a5(6, 0xa2, 0x24);
    f03a5(7, 0xa2, 0x24);
    f03a5(0x138, 0xa2, 0x24);
    f03a5(0x139, 0xa2, 0x24);
    f01ce(9);
    f03a2(1, 0xd, 0x13e);
    f03a2(1, 0xe, 0x13e);
    f03a2(6, 0xa1, 0x134);
    f03a2(1, 0xc6, 0x13e);
    f03a8(1, 0xd, 5, 0xba);
    f03a8(0x13a, 0xd, 5, 0xba);
    f01ce(save);
}
