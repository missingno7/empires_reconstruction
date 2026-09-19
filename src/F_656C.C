/* F_656C -- load one indexed record from the open data file and hand it to
   whichever unpacker its header byte names.  Entry 1666C, 517 bytes, 950
   activations.  One unsigned parameter that packs a directory index in its
   top four bits and a record number in the low twelve.

   Read off assets/AEPROG.EXE (ndisasm -b16 -o 0x656c):

   6574  8B7E04 B10C D3EF     d = p >> 12: `shr` (UNSIGNED) with the count in
                               CL, so p is unsigned and the shift is not an
                               arithmetic one.
   6581  816604FF0F           p &= 0xFFF -- the parameter itself is the
                               destination, so the source assigns to it.
   658C  the do-while top. gb3e is cleared here, inside the loop, and tested
         twice: once at 6609 to guard the retry block and once at 6631 as the
         loop condition.  The back edge is 175 bytes, past a short jump's
         reach, so TC 2.0 writes it as `jz over / jmp top` (7403 E951FF).
   659A  D1E0 D1E0 33D2 52 50 p*4 zero-extended into DX:AX and pushed high
                               word first: ONE long argument built from an
                               unsigned int, which is what `(long)(p * 4)`
                               compiles to.  A signed int would have been
                               widened with CWD, and a long-typed p would not
                               have needed the widening at all.
   65B3  16 8D46F4 50         push ss / lea / push -- &o1 as a far pointer to
                               a local, the compact model's default.
   65D5  8B76F8 2B76F4        the size is a SIXTEEN-bit subtraction of the two
                               longs' low words: `(int)o2 - (int)o1`.  Writing
                               `(int)(o2 - o1)` would have emitted the 32-bit
                               sub/sbb pair first and then truncated.
   65EF  FF36CCC5 FF36CAC5    a far POINTER global pushed as segment word then
                               offset word (gc5cc:gc5ca), against the
                               `push ds / mov ax,offset / push ax` an array
                               name would have produced.
   6656  4E 4E                s -= 2 as two DECs on the register variable.
   locals            TC 2.0 lays locals out in REVERSE declaration order
                              upward from bp, each aligned to even: sv (int) at
                              bp-02, fl (char) at bp-03, o2 (long) at bp-08
                              (bp-07 rounded down), o1 at bp-0C, frame 0x0C.
   6658  F646FD02 743A F646FD01 7434
                              `if ((fl & 2) && (fl & 1))` -- both tests jump
                               to the same else arm, the shape && produces.
   670D  A0CBC0 98 0BC0 7431 3D0100 743D 3D4700 7402 EB45
                              a SWITCH on a char: one load, sign-extended,
                               then `or ax,ax` for the zero case and `cmp ax`
                               for the others, dispatched in ASCENDING case
                               order (0, 1, 0x47) while the bodies stay in
                               source order (0x47 at 6721, 0 at 6746, 1 at
                               6757) -- the same ordering F_5382 carries.
   6764  EB00                 the last arm's break, displacement zero.
   6769  EB00                 `return s;` jumps to the epilogue that already
                               follows it. */

extern int  f6dcc(), f6d86(), f6771(), f67dc(), f6f4b(), f6eff();
extern int  lseek(), read(), close(), memmove(), f86c9(), f6266();

extern int  gb3e, gb40;                 /* DS:0B3E, DS:0B40 */
extern char far *gb31;                  /* DS:0B31, segment at DS:0B33 */
struct R { char b[0x23]; };             /* 35-byte directory record */
extern struct R ga5e[];                 /* DS:0A5E */
extern char gb2a[];                     /* DS:0B2A */
extern int  gc0c9;                      /* DS:C0C9 -- the open handle */
extern char gc0cb;                      /* DS:C0CB */
extern char gbfcd;                      /* DS:BFCD */
extern char far *gc5be;                 /* DS:C5BE, segment at DS:C5C0 */
extern char far *gc5c6;                 /* DS:C5C6, segment at DS:C5C8 */
extern char far *gc5ca;                 /* DS:C5CA, segment at DS:C5CC */

f656c(p)
unsigned p;
{
    long o1;                            /* bp-0C */
    long o2;                            /* bp-08 */
    char fl;                            /* bp-03 */
    int sv;                             /* bp-02 */
    register int s, d;                  /* si, di */

    d = p >> 12;
    sv = gb40;
    p &= 0xfff;
    gb40 = 1;
    do {
        gb3e = 0;
        f6266(d);
        lseek(gc0c9, (long)(p * 4), 0);
        read(gc0c9, &o1, 4);
        read(gc0c9, &o2, 4);
        s = (int)o2 - (int)o1;
        lseek(gc0c9, o1, 0);
        read(gc0c9, gc5ca, s);
        close(gc0c9);
        if (gb3e != 0) {
            gb31 = (char far *)&ga5e[d];
            f86c9(gb2a);
        }
    } while (gb3e != 0);
    gb40 = sv;
    gc0cb = gc5ca[0];
    fl = gc5ca[1];
    s -= 2;
    if ((fl & 2) && (fl & 1)) {
        s = f6dcc(gc5c6, gc5be, s);
        s = f6d86(gc5be, gc5c6, s);
    } else if (fl & 2) {
        s = f6dcc(gc5c6, gc5be, s);
        memmove(gc5c6, gc5be, s);
    } else if (fl & 1) {
        s = f6d86(gc5c6, gc5be, s);
        memmove(gc5c6, gc5be, s);
    }
    if (gbfcd != 5) {
        switch (gc0cb) {
        case 0x47:
            if (gbfcd == 2)
                f6f4b(gc5c6);
            else
                f6eff(gc5c6);
            break;
        case 0:
            f6771(gc5c6, s);
            break;
        case 1:
            f67dc(gc5c6);
            break;
        }
    }
    asm sti;
    return s;
}
