/* cclib.c -- Turbo C 2.0 CC.LIB semantics: movmem/setmem, the exact rand()
 * LCG, and itoa/ltoa/ultoa digit formatting.  See cclib.h for the mapping
 * rationale and docs/portable/funcs-inventory.md sec 6 for the full
 * call-site table this implements.
 */
#include "cclib.h"

/* ---- movmem / setmem --------------------------------------------------
 * CC.LIB's movmem handles overlap (it is literally memmove under a
 * swapped argument order); setmem is memset with dst/count swapped
 * relative to setmem's own (dst, n, val) order -- already matches memset's
 * (dst, val, n) once val/n are placed correctly below. */
void cc_movmem(const void *src, void *dst, dos_uint n)
{
    memmove(dst, src, n);
}

void cc_setmem(void *dst, dos_uint n, dos_char val)
{
    memset(dst, (unsigned char)val, n);
}

/* ---- rand() / srand(seed): Turbo C 2.0's exact 32-bit LCG -------------
 * src/LIB_RAND.C (the historical CC.LIB contribution, reconstructed from
 * the compact-model object code) is byte-for-byte:
 *   static long state = 1L;
 *   void srand(seed) unsigned seed; { state = seed; }
 *   int rand() { state = state * 0x015A4E35L + 1L; return (int)(state >> 16) & 0x7fff; }
 * Reimplemented here with fixed-width dos_* types (state as dos_ulong: the
 * multiply/add wrap at 32 bits exactly like the historical `long`, and the
 * `>> 16` then `& 0x7fff` is the same unsigned-shift-then-mask either way).
 *
 * First 5 values for the default seed (1), computed independently in
 * Python for this comment (also asserted by test_cclib.c):
 *   seed=1
 *   for i in range(5):
 *       seed = (seed * 0x015A4E35 + 1) & 0xFFFFFFFF
 *       print((seed >> 16) & 0x7fff)
 *   -> 346, 130, 10982, 1090, 11656
 */
static dos_ulong s_rand_state = 1UL;

dos_int cc_rand(void)
{
    s_rand_state = s_rand_state * 0x015A4E35UL + 1UL;
    return (dos_int)((s_rand_state >> 16) & 0x7fffUL);
}

void cc_srand(dos_uint seed)
{
    s_rand_state = seed;
}

/* ---- itoa/ltoa/ultoa ---------------------------------------------------
 * Turbo C's non-ANSI itoa/ltoa/ultoa: lowercase digits for radix > 10;
 * '-' prefix only for itoa/ltoa's SIGNED negative value in radix 10 (every
 * other radix -- including a negative value in a non-10 radix -- formats
 * the value's raw bit pattern as unsigned, matching Turbo C's behaviour of
 * printing hex/octal/binary as the two's-complement bit pattern rather
 * than a signed magnitude).  ultoa's value is unsigned to begin with, so
 * it always takes the unsigned path.
 */
static const char DIGITS[] = "0123456789abcdefghijklmnopqrstuvwxyz";

static dos_char *format_unsigned(dos_ulong v, dos_char *buf, dos_int radix)
{
    dos_char tmp[34]; /* 32 bits in radix 2, +1 for safety, +1 NUL */
    int i = 0;
    dos_char *p = buf;

    if (v == 0) {
        tmp[i++] = '0';
    } else {
        while (v != 0) {
            tmp[i++] = (dos_char)DIGITS[v % (dos_ulong)radix];
            v /= (dos_ulong)radix;
        }
    }
    while (i > 0) {
        *p++ = tmp[--i];
    }
    *p = 0;
    return buf;
}

dos_char *cc_ultoa(dos_ulong value, dos_char *buf, dos_int radix)
{
    format_unsigned(value, buf, radix);
    return buf;
}

dos_char *cc_ltoa(dos_long value, dos_char *buf, dos_int radix)
{
    dos_char *p = buf;
    if (radix == 10 && value < 0) {
        *p++ = '-';
        /* -value as dos_ulong: safe even at LONG_MIN (magnitude does not
         * fit in a signed dos_long, but does fit in dos_ulong). */
        format_unsigned((dos_ulong)(0UL - (dos_ulong)value), p, radix);
    } else {
        format_unsigned((dos_ulong)value, buf, radix);
    }
    return buf;
}

dos_char *cc_itoa(dos_int value, dos_char *buf, dos_int radix)
{
    dos_char *p = buf;
    if (radix == 10 && value < 0) {
        /* Unsigned wraparound negation at 16 bits gives the correct
         * magnitude even at value == INT16_MIN (whose magnitude, 32768,
         * does not fit back in a signed 16-bit dos_int but does fit in
         * dos_uint) -- same trick used at 32 bits in cc_ltoa above. */
        dos_uint mag = (dos_uint)(0u - (dos_uint)value);
        *p++ = '-';
        format_unsigned((dos_ulong)mag, p, radix);
    } else {
        format_unsigned((dos_ulong)(dos_uint)value, buf, radix);
    }
    return buf;
}
