/* test_cclib.c -- unit tests for portable/include/cclib.h /
 * portable/compat/cclib.c (Turbo C 2.0 CC.LIB semantics).
 */
#include "cclib.h"

#include <stdio.h>
#include <string.h>

static int s_failures = 0;

#define CHECK(cond) \
    do { \
        if (!(cond)) { \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
            s_failures++; \
        } \
    } while (0)

/* (const char *) casts below: dos_char is int8_t (signed char) per
 * dos_types.h's "Turbo C char is signed by default" rule, so itoa/ltoa/
 * ultoa's dos_char * return is a distinct type from <string.h>'s plain
 * `char *` as far as MSVC's C5292 is concerned, even though it is the
 * same width/representation -- a normal, expected friction point at every
 * dos_char / char boundary in this codebase, not a real type error. */
#define CHECK_STR(actual, expected) \
    do { \
        if (strcmp((const char *)(actual), (expected)) != 0) { \
            fprintf(stderr, "FAIL %s:%d: got \"%s\", want \"%s\"\n", \
                    __FILE__, __LINE__, (const char *)(actual), (expected)); \
            s_failures++; \
        } \
    } while (0)

/* Turbo C 2.0's LCG (src/LIB_RAND.C):
 *   state = state * 0x015A4E35 + 1; return (state >> 16) & 0x7fff;
 * with the default initial seed 1.  First 5 values, computed independently
 * in Python for this test:
 *   seed = 1
 *   for i in range(5):
 *       seed = (seed * 0x015A4E35 + 1) & 0xFFFFFFFF
 *       print((seed >> 16) & 0x7fff)
 *   -> 346, 130, 10982, 1090, 11656
 */
static void test_rand_sequence_default_seed(void)
{
    static const dos_int expected[5] = {346, 130, 10982, 1090, 11656};
    int i;
    for (i = 0; i < 5; i++) {
        CHECK(rand() == expected[i]);
    }
}

/* srand(1) must reproduce the exact same sequence as the untouched default
 * (Turbo C's C startup behaves as if srand(1) had already been called). */
static void test_srand_reseed_matches_default(void)
{
    static const dos_int expected[5] = {346, 130, 10982, 1090, 11656};
    int i;
    srand(1);
    for (i = 0; i < 5; i++) {
        CHECK(rand() == expected[i]);
    }
}

/* A different seed must diverge from the seed-1 sequence (sanity check
 * that srand() actually reseeds cc_rand's state and not some other
 * generator). */
static void test_srand_different_seed_diverges(void)
{
    srand(42);
    CHECK(rand() != 346);
}

static void test_itoa_basic(void)
{
    dos_char buf[34];
    CHECK_STR(itoa(0, buf, 10), "0");
    CHECK_STR(itoa(12345, buf, 10), "12345");
    CHECK_STR(itoa(-12345, buf, 10), "-12345");
    CHECK_STR(itoa(-1, buf, 10), "-1");
}

static void test_itoa_radix(void)
{
    dos_char buf[34];
    CHECK_STR(itoa(255, buf, 16), "ff");
    CHECK_STR(itoa(8, buf, 2), "1000");
    CHECK_STR(itoa(8, buf, 8), "10");
    /* Non-10 radix formats the raw bit pattern as unsigned, matching
     * Turbo C -- a negative value in hex is its two's-complement form,
     * not a '-' prefix. */
    CHECK_STR(itoa(-1, buf, 16), "ffff");
}

static void test_itoa_int16_min(void)
{
    /* -32768's magnitude (32768) does not fit back in a signed 16-bit
     * dos_int -- exercises the unsigned-wraparound-negation path. */
    dos_char buf[34];
    CHECK_STR(itoa(-32768, buf, 10), "-32768");
}

static void test_ltoa_basic(void)
{
    dos_char buf[34];
    CHECK_STR(ltoa(0L, buf, 10), "0");
    CHECK_STR(ltoa(123456789L, buf, 10), "123456789");
    CHECK_STR(ltoa(-123456789L, buf, 10), "-123456789");
}

static void test_ultoa_basic(void)
{
    dos_char buf[34];
    CHECK_STR(ultoa(0UL, buf, 10), "0");
    CHECK_STR(ultoa(4294967295UL, buf, 10), "4294967295");
    CHECK_STR(ultoa(0xDEADBEEFUL, buf, 16), "deadbeef");
    CHECK_STR(ultoa(255UL, buf, 2), "11111111");
}

static void test_movmem_overlap_forward(void)
{
    /* dst > src, overlapping ranges: a naive byte-forward copy would
     * corrupt the tail; movmem (memmove semantics) must not. */
    char buf[10] = "ABCDEFGHI";
    movmem(buf, buf + 2, 6); /* shift "ABCDEF" right by 2 -> "ABABCDEFI" */
    CHECK(memcmp(buf, "ABABCDEFI", 9) == 0);
}

static void test_movmem_overlap_backward(void)
{
    /* src > dst: shift left. */
    char buf[10] = "ABCDEFGHI";
    movmem(buf + 2, buf, 6); /* "CDEFGH" moved to the front -> "CDEFGHGHI" */
    CHECK(memcmp(buf, "CDEFGHGHI", 9) == 0);
}

static void test_setmem_basic(void)
{
    char buf[8];
    setmem(buf, sizeof(buf), 'x');
    CHECK(memcmp(buf, "xxxxxxxx", 8) == 0);
}

int main(void)
{
    test_rand_sequence_default_seed();
    test_srand_reseed_matches_default();
    test_srand_different_seed_diverges();
    test_itoa_basic();
    test_itoa_radix();
    test_itoa_int16_min();
    test_ltoa_basic();
    test_ultoa_basic();
    test_movmem_overlap_forward();
    test_movmem_overlap_backward();
    test_setmem_basic();

    if (s_failures) {
        fprintf(stderr, "test_cclib: %d check(s) failed\n", s_failures);
        return 1;
    }
    printf("test_cclib: all checks passed\n");
    return 0;
}
