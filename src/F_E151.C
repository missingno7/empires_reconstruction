/* F_E151 -- unit-order dispatch.  Plain C switch: TC 2.01 emits the 18-entry
   word table into _TEXT right after the jmp word ptr cs:[bx+t], and lays the
   case bodies out in source order, which is what fixes the case order here. */
extern void fe420();
extern void fe262();
extern void fe1f2();
extern void fe27b();
extern void fe2d6();
extern void fe324();
extern void fe372();
extern void fe44b();

void fe151(u, k)
int u, k;
{
    switch (k) {
    case 14: case 15: case 17:
        fe420(); break;
    case 16:
        fe262(); break;
    case 0: case 8:
        fe1f2(u); break;
    case 2: case 12:
        fe27b(u); break;
    case 3: case 6:
        fe2d6(u); break;
    case 4: case 7:
        fe324(u); break;
    case 1: case 5: case 9: case 10: case 11:
        fe372(u); break;
    case 13:
        fe44b(u); break;
    }
}
