/* F_DD72 -- retune one voice: two fe095 calls for the two bytes of the
   2-byte record at DS:2FD2, then reprogram the voice through fe48a. */
extern void fe095();
extern void fe48a();
extern char gc6b6[];                    /* DS:C6B6, byte per voice */
struct rdd72 { char a, b; };
extern struct rdd72 g2fd2[];            /* DS:2FD2, 2 bytes per voice */

void fdd72(v)
register int v;
{
    fe095(g2fd2[v].a, 7, 0xa);
    fe095(g2fd2[v].b, 7, 0xa);
    fe48a(v, gc6b6[v], 0);
}
