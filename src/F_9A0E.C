/* F_9A0E -- run one entry of the 18h-stride script table g0DCC: notify
   f99E2/f984C, and either draw the "selected" bitmap out of the far blob
   gC5C6 at the offset its own header word 122h names (when the entry is the
   current one and gC354 is clear), or draw the bitmap the entry's byte names
   and dispatch through the handler table g12A1 by its signed byte. */
struct HDR { char pad[0x122]; int f122; };
struct P2 { unsigned char a; char b; };
struct R18 { struct P2 e[12]; };
extern struct R18 g0dcc[];
extern int gc359, gc354, gc35d, gc34e, gc350;
extern char far *gc5c6;
extern void (*g12a1[])(void);
extern void f99e2(int n);
extern void f984c(void);
extern void f01ce(int n);
extern void f03cf(int x, int y, char far *p);

/*@PUB _f9a0e*/
void f9a0e(int n, int j)
{
    f99e2(n);
    f984c();
    if (n == gc359 && gc354 == 0) {
        f01ce(5);
        f03cf(gc34e, gc350, gc5c6 + ((struct HDR far *) gc5c6)->f122 + 2);
        f01ce(0);
    } else {
        f03cf(0, 0x168,
              gc5c6 + ((int far *) gc5c6)[g0dcc[gc35d].e[j].a] + 2);
        (*g12a1[g0dcc[gc35d].e[j].b])();
    }
}
