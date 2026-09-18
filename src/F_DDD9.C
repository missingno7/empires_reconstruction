/* F_DDD9 -- the tuning arithmetic: turn a note and an octave into a divisor,
   entirely in `long`.  Every multiply that keeps its high word is a CC.LIB
   LXMUL@ far call and every divide is a LDIV@ far call, so the constants
   52088, 25, 9 and 111875 are long constants; `mul` IMMEDIATELY followed by
   `cwd` is instead an int multiply whose RESULT is widened (rule 5), which is
   what the first two statements do.  Three longs in a 0x0C frame, in reverse
   declaration order (rule 1): the one written first is declared last. */
long fddd9(a, b)
int a;
int b;
{
    long t;
    long u;
    long l;

    l = (long) (b * 100);
    t = ((long) (a * 6) + l) * 52088L;
    t = t / (l * 25);
    u = t << 14;
    u = u * 9;
    u = u / 111875L;
    return (u);
}
