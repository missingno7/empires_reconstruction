/* F_9259 -- release the two far blocks at DS:0DC8 and DS:0DC4 if they are
   held.  TC 2.0 tests a far pointer against NULL by OR-ing its two halves
   (rule 16), and clearing one writes the SEGMENT half first.  The callee is
   spelled BY THE CONVENTION: the image calls IP 0F3F7h, which is the FFREE
   module's base and NOT `_farfree` -- the declared binding puts farfree at
   0F6C3h, +0x2CC into the same module. */
extern void ff3f7();
extern char far *gdc8;                  /* DS:0DC8 offset, DS:0DCA segment */
extern char far *gdc4;                  /* DS:0DC4 offset, DS:0DC6 segment */

void f9259()
{
    if (gdc8) {
        ff3f7(gdc8);
        gdc8 = 0;
    }
    if (gdc4) {
        ff3f7(gdc4);
        gdc4 = 0;
    }
}
