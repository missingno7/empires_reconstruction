/* F_E0C0 -- copy 13 bytes out of a caller's stride-2 table into the voice's
   14-byte record, then store the masked mode in the record's last byte and
   re-arm the voice through fe1c4. */
extern void fe1c4();
struct r14 { char c0, c1, c2, c3, c4, c5, c6;
             char c7, c8, c9, c10, c11, c12, c13; };
extern struct r14 gc91b[];              /* DS:C91B, 14 bytes per voice */

void fe0c0(v, q, w)
int v;
char far *q;
int w;
{
    char far *p;
    register int i;

    for (i = 0, p = (char far *)&gc91b[v]; i < 13; i++) {
        *p = *q;
        q += 2;
        p++;
    }
    *p = w &= 3;
    fe1c4(v);
}
