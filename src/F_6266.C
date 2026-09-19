/* F_6266 -- open the data file named in directory row i, retry once with the
   name's first letter case-folded, and report through gb31/f86c9 if neither
   attempt matches the recorded stamp.  Entry 16366, 468 bytes.

   627F  893E3E0B            `ok = 0; gb3e = ok;` -- TC 2.0 stores the
              register variable STRAIGHT to memory.  Written as the chain
              `gb3e = ok = 0;` it inserts `mov ax,di` first (attempt 2's
              first difference, extent offset 25: original 89, candidate 8B).
   628D  05220A 8CDA 52 50   the name argument is the ROW of a 2D char array,
              `ga22[i]`, which is one complete far-pointer operand: offset
              computed in AX, base added into AX, segment last.  Written as
              `(char far *)ga22 + i * 0x10 + K` instead, TC reassociates the
              two int addends and defers the base to the very end (attempt 3's
              first difference, extent offset 39: original 05, candidate 50).
   62A0..62B4  the relational is multiplied: `(ga22[i][0] > 0x42) * 3` gives
              the 0/1 branch pair and then `mov dx,3; mul dx`; `cond ? 3 : 0`
              would have written 3 straight into AX with no mul.  The far
              pointer is PUSHED before that addend is computed and POPPED
              back to add it, which is TC's shape for a pointer argument
              whose int addend needs branches of its own.
   62D7..6303  A && B && C && D under a `!`: each failing test jumps INTO the
              block and the all-pass path jumps over it.
   6338  B042 2A46F7 0441   `c = 0x42 - c + 0x41` stays in AL: char
              arithmetic in a byte register when operands and destination are
              all chars, and the two constants are NOT folded.
   63DD / 6403  gb31 = (char far *)&ga5e[i] twice, both in the
              array-of-0x23-byte-struct shape (mov bx,ax; add bx,offset;
              push ds; pop es) that F_656C's negative test established.
   62F1  8B97540A 8B87520A   ga52[i] is a plain long array indexed by a
              register: near DS addressing, base folded into the
              displacement, no segment materialised. */

struct R { char b[0x23]; };             /* 35 bytes, `mov dx,0x23; mul dx` */

extern int open(), read(), close(), f86c9();

extern int  gb3e, gb40, gc0c9;          /* DS:0B3E, DS:0B40, DS:C0C9 */
extern char far *gb31;                  /* DS:0B31, segment at DS:0B33 */
extern char gb2a[];                     /* DS:0B2A */
extern char gbfcc;                      /* DS:BFCC */
extern char ga22[][16];                 /* DS:0A22 -- 16-byte name rows */
extern long ga52[];                     /* DS:0A52 */
extern struct R ga5e[];                 /* DS:0A5E */

f6266(i)
register int i;
{
    char c;                             /* bp-09 */
    int sv;                             /* bp-08 */
    long stamp;                         /* bp-06 */
    int n;                              /* bp-02 */
    register int ok;                    /* di */

    sv = gb40;
    gb40 = 1;
    do {
        ok = 0;
        gb3e = ok;
        gc0c9 = open(ga22[i] + (ga22[i][0] > 0x42) * 3, 0x8004);
        n = read(gc0c9, &stamp, 4);
        if (!(gc0c9 >= 0 && gb3e == 0 && n >= 4 && ga52[i] == stamp)) {
            if (gc0c9 >= 0)
                close(gc0c9);
            if (gbfcc > 1 && (c = ga22[i][0]) <= 0x42) {
                c = 0x42 - c + 0x41;
                ga22[i][0] = c;
                gb3e = 0;
                gc0c9 = open(ga22[i], 0x8004);
                n = read(gc0c9, &stamp, 4);
                if (!(gc0c9 >= 0 && gb3e == 0 && n >= 4
                      && ga52[i] == stamp)) {
                    if (gc0c9 >= 0)
                        close(gc0c9);
                    ga22[i][0] = 0x42 - c + 0x41;
                    gb31 = (char far *)&ga5e[i];
                    f86c9(gb2a);
                    ok = 1;
                }
            } else {
                gb31 = (char far *)&ga5e[i];
                f86c9(gb2a);
                ok = 1;
            }
        }
    } while (ok);
    gb40 = sv;
}
