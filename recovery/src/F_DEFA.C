/* F_DEFA -- reset the mixer: hand each of the 25 channel records to F_DE7E
   with a rising 100ths accumulator, point the eleven far slots at the first
   record, and fill the 8x12 row/column index pair tables.  Four stack locals,
   laid out in reverse declaration order upward from BP (rule 1): the one
   written first, `w`, is declared last at bp-02.  The record argument is the
   ROW OF A 2D ARRAY (rule 2's row shape): `ds` is pushed, the index scaled and
   the array base added into AX LAST.  A one-dimensional `gc6c3 + i * 24`
   instead builds a complete far pointer first and is four bytes longer.  The accumulator is zeroed
   by a CHAINED assignment -- `a = i = 0` routes through AX (rule 4), where
   `a = 0` on its own would have stored DI straight to bp-04. */
extern void music_build_octave_table();
extern char far *gca24[];               /* DS:CA24 */
extern unsigned gca6d[];                /* DS:CA6D */
extern unsigned char gc5ea[];           /* DS:C5EA */
extern unsigned char gc64a[];           /* DS:C64A */
extern unsigned char gc6c3[][24];       /* DS:C6C3 */

void music_reset_tuning_tables()
{
    unsigned r;
    unsigned c;
    unsigned a;
    unsigned w;
    register unsigned p;
    register unsigned i;

    w = 4;
    for (a = i = 0; i < 25; i++, a += w)
        music_build_octave_table(gc6c3[i], a, 100);
    for (r = 0; r < 11; r++) {
        gca24[r] = gc6c3[0];
        gca6d[r] = 0;
    }
    for (p = 0, r = 0; r < 8; r++)
        for (c = 0; c < 12; c++, p++) {
            gc5ea[p] = r;
            gc64a[p] = c;
        }
}
