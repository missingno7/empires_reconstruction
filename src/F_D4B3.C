/* F_D4B3 -- toggle one bit in the current record's mask and, if it was set,
   look up its string and hand it on.  27-byte records at gc470. */
struct rec {
    char pad[17];
    int f11;
    int f13;
    char tail[6];
};

extern struct rec gc470[];
extern unsigned g13ed;
extern int f684a(), f86c9(), ff6c3();
extern char far *g235d;
extern int g2356;

void fd4b3(int n)
{
    char far *p;

    if (gc470[g13ed].f11 == 0)
        return;
    if (gc470[g13ed].f13 & (1 << n)) {
        gc470[g13ed].f13 ^= (1 << n);
        f684a(0x101d, &p);
        g235d = p + ((int far *) p)[n] + 2;
        f86c9(&g2356);
        ff6c3(p);
    }
}
