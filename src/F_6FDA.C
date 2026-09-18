/* F_6FDA -- open the panel: blit its backdrop, draw the five widgets, frame
   it.  gc0ee is a far pointer whose first word is its own length. */
extern int gc0fa, gc0fc, g0b83;
extern char far *gc0ee;
extern int f03c9(), f7202(), f7298(), f738a(), f7417(), f7443(), f039f(), f747b();

void f6fda(void)
{
    gc0fa = 0;
    g0b83 = 1;
    gc0fc = 1;
    f03c9(6, 0xa2, gc0ee + *(int far *) gc0ee + 2);
    f7202();
    f7298();
    f738a();
    f7417();
    f7443();
    f039f(6, 0xa2, 0x134, 0x24);
    f747b();
    gc0fa = 1;
}
