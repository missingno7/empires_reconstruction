/* F_5382 -- the intro's animation-step service.  Entry 15482, 473 bytes,
   executed 40,061,167 times on the corpus (the hottest function the intro
   chapter owns).  Eleven parameters, eleven int locals, two register
   variables (SI = the slot index, DI = x).

   Shape, read off assets/AEPROG.EXE (ndisasm -b16 -o 0x5382):

     bp+04  struct E far *ev   16-byte script slots (`mov cl,4; shl ax,cl`)
     bp+08  int  step
     bp+0A  struct P far *q    4-byte pairs (`shl ax,1; shl ax,1`)
     bp+0E  int far *idx
     bp+12  unsigned long far *when
     bp+16  int  dx0
     bp+18  int  dy0
     bp+1A  int far *ph
     bp+1E  int far *pi
     bp+22  int far *pj
     bp+26  int far *pk

   5382  55 8BEC 83EC16 5657   prologue, 0x16 = eleven int locals
   538A  C45E12 268B5702 268B07 3B16780B 720B 7706 3B06760B 7603 E9B001
                                *when > gb76 -- the 32-bit UNSIGNED compare
                                (jc / ja / jna), so both sides are unsigned
                                long; greater means jmp to the epilogue.
   53A5  C45E0E 268B37 26FF07  s = (*idx)++   (value first, then inc memory)
   53AE..543D                  eight separate statements, each recomputing
                                s*16 and reloading ev: TC 2.0 does no CSE.
                                [bp-0xE] is written and never read again.
   5440  8B46EA 3DFEFF 7432 3DFFFF 7402 EB36
                                a SWITCH, not an if-chain: the selector is
                                loaded ONCE into ax and each arm is a
                                `cmp ax,imm / jz`, and the chain is emitted
                                in ASCENDING case order (-2 before -1) while
                                the BODIES stay in source order (-1 at 544F,
                                -2 at 547A, default at 5485).  The last
                                arm's break is the give-away `EB00` at 5553,
                                a jump of displacement zero to the switch's
                                end, which here is also the epilogue.
   548B  8B46F6 F76608 99      n * step then CWD: the `mul` is a 16x16 int
                                multiply whose high word is thrown away by
                                the sign-extension that widens the int
                                result to the long that is added to gb76.
   54EA  ...26FF7702 26FF37    q[cmd] is pushed as TWO words from ONE address
                                computation -- a 4-byte STRUCT passed BY
                                VALUE, not two separate int arguments, which
                                would each have recomputed les bx / add bx.
                                `add sp,0xA` = 5 words = 4 arguments. */

struct E {                              /* 16 bytes, `mov cl,4; shl ax,cl` */
    int f0, f2, f4, f6, f8, fa, fc, fe;
};

struct P {                              /* 4 bytes, `shl ax,1; shl ax,1` */
    int a, b;
};

extern void f03b4(), f039f(), f03cc(), fcb48(), fcaf1();

extern unsigned long gb76;              /* DS:0B76 */

f5382(ev, step, q, idx, when, dx0, dy0, ph, pi, pj, pk)
struct E far *ev;
int step;
struct P far *q;
int far *idx;
unsigned long far *when;
int dx0, dy0;
int far *ph, far *pi, far *pj, far *pk;
{
    int cmd;                            /* bp-16 */
    int y;                              /* bp-14 */
    int w;                              /* bp-12 */
    int h;                              /* bp-10 */
    int t;                              /* bp-0E, written, never read */
    int c;                              /* bp-0C */
    int n;                              /* bp-0A */
    int ox;                             /* bp-08 */
    int oy;                             /* bp-06 */
    int ow;                             /* bp-04 */
    int oh;                             /* bp-02 */
    register int s, x;                  /* si, di */

    if (*when > gb76) return;
    s = (*idx)++;
    cmd = ev[s].f0;
    x = ev[s].f2 + dx0;
    y = ev[s].f4 + dy0;
    w = ev[s].f6;
    h = ev[s].f8;
    t = ev[s].fa;
    c = ev[s].fc;
    n = ev[s].fe;
    switch (cmd) {
    case -1:
        f03b4(x, y + 0xc8, w, h, x, y);
        f039f(x, y, w, h);
        break;
    case -2:
        fcb48();
        fcaf1(x);
        break;
    default:
        if (n != 0)
            *when = n * step + gb76;
        ox = *ph;
        oy = *pi;
        ow = *pj;
        oh = *pk;
        if (ow != 0)
            f03b4(ox, oy + 0xc8, ow, oh, ox, oy);
        f03cc(x, y, q[cmd], c);
        if (ow != 0)
            f039f(ox, oy, ow, oh);
        f039f(x, y, w, h);
        *ph = x;
        *pi = y;
        *pj = w;
        *pk = h;
        break;
    }
}
