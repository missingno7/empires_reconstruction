/* F_DE7E -- fill twelve divisors of one octave into the caller's word array,
   each 106/100 of the one before it, rounded as `(unsigned)l + 4 >> 3`.  The
   far pointer PARAMETER is advanced in place (`add word ptr [bp+4],2`), so
   the source increments the parameter rather than a local copy. */
extern long fddd9();

void fde7e(p, a, b)
unsigned far *p;
int a;
int b;
{
    long l;
    register int i;

    *p = ((unsigned) (l = fddd9(a, b)) + 4) >> 3;
    p++;
    for (i = 1; i < 12; i++) {
        l = l * 106;
        *p = ((unsigned) (l = l / 100) + 4) >> 3;
        p++;
    }
}
