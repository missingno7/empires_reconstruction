/* F_E095 -- store one byte into the voice's 14-byte record at a runtime
   member index and re-emit that member through fe151. */
extern void fe151();
struct r14a { char c[14]; };
extern struct r14a gc91b[];             /* DS:C91B, 14 bytes per voice */

void fe095(v, k, val)
int v;
int k;
char val;
{
    gc91b[v].c[k] = val;
    fe151(v, k);
}
