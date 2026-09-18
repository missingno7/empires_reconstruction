/* F_DA66 -- draw a slot's two glyphs: the pair at record offset 0x34 with the
   record itself, and the pair's second half with the sub-record at 0x1A.
   Entry 1DB66, 113 bytes.

   DA6E  C45E06 83C334 8C46FE 895EFC   p = a + 0x34: `les` loads the far
              pointer whole, the constant lands on the OFFSET, the store
              writes the SEGMENT word first -- the same shape F_D8F0 carries
              at D8F7.
   DA80  8346FC02            `p += 2` touches only the offset word.
   DAA0  D1E3 81C3D22F 1E 07 268A07   g2fd2[i].a -- a 2-byte struct through an
              array with a runtime index, so `shl bx,1` for the stride and the
              far form built OFFSET first.
   The two locals sit at bp-08 and bp-04, so the one written FIRST (p) is
   declared LAST: TC 2.0 lays locals out in reverse declaration order,
   upward from bp. */

struct P2 { char a, b; };

extern int fe0c0();

extern struct P2 g2fd2[];               /* DS:2FD2 */

fda66(i, a)
int i;
char far *a;
{
    char far *q;                        /* bp-08 */
    char far *p;                        /* bp-04 */
    register int s, d;                  /* si, di */

    p = a + 0x34;
    s = *(int far *)p;
    p += 2;
    d = *(int far *)p;
    q = a + 0x1a;
    fe0c0(g2fd2[i].a, a, s);
    fe0c0(g2fd2[i].b, q, d);
}
