/* test_asm_drawqbuf.c -- unit tests for portable/game/asm_drawqbuf.c
 * (draw_queue_reset, draw_queue_append -- the port of asm/DRAWQBUF.ASM).
 *
 * g2f30 (DS:2F30, generated) is a real global; these tests read/write it
 * directly, matching asm_drawqbuf.h's documented layout: g2f30[0] = record
 * count, then 5-byte records at g2f30[1 + count*5].
 */
#include "game.h"
#include "asm_drawqbuf.h"

#include <stdio.h>
#include <string.h>

static int g_failures = 0;

static void fail(const char *test, const char *what)
{
    fprintf(stderr, "test_asm_drawqbuf: FAIL %s: %s\n", test, what);
    g_failures++;
}

static void check(const char *test, int cond, const char *what)
{
    if (!cond) fail(test, what);
}

static void test_reset_zeroes_count(void)
{
    const char *t = "reset_zeroes_count";
    memset(g2f30, 0xCC, sizeof g2f30); /* poison the whole buffer */
    draw_queue_reset();
    check(t, g2f30[0] == 0, "draw_queue_reset must zero g2f30[0] and nothing else it doesn't own");
    check(t, g2f30[1] == 0xCC, "draw_queue_reset must not touch record storage, only the count byte");
}

static void test_append_basic_packing(void)
{
    const char *t = "append_basic_packing";
    memset(g2f30, 0, sizeof g2f30);

    draw_queue_append(7, 0x1234, 0x5678, 0x9abc, 0x0def);

    check(t, g2f30[0] == 1, "count must be incremented to 1 after one append");
    /* record 0 at g2f30[1..5]: byte0=attr, byte1=x lo, byte2=y lo, byte3=color lo, byte4=height lo */
    check(t, (dos_uchar)g2f30[1] == 7, "byte0 must be the attr argument's low byte");
    check(t, (dos_uchar)g2f30[2] == 0x34, "byte1 must be x's low byte only (0x1234 truncated to 0x34)");
    check(t, (dos_uchar)g2f30[3] == 0x78, "byte2 must be y's low byte only (0x5678 truncated to 0x78)");
    check(t, (dos_uchar)g2f30[4] == 0xbc, "byte3 must be color's low byte only (0x9abc truncated to 0xbc)");
    check(t, (dos_uchar)g2f30[5] == 0xef, "byte4 must be height's low byte only (0x0def truncated to 0xef)");
}

static void test_append_sequential_records(void)
{
    const char *t = "append_sequential_records";
    draw_queue_reset();

    draw_queue_append(1, 10, 20, 30, 40);
    draw_queue_append(2, 11, 21, 31, 41);
    draw_queue_append(3, 12, 22, 32, 42);

    check(t, g2f30[0] == 3, "three appends must leave count == 3");
    check(t, (dos_uchar)g2f30[1] == 1 && (dos_uchar)g2f30[6] == 2 && (dos_uchar)g2f30[11] == 3,
          "each record's attr byte must land DRAW_QUEUE_APPEND_RECORD_BYTES apart, in append order");
    check(t, (dos_uchar)g2f30[2] == 10 && (dos_uchar)g2f30[3] == 20, "record 0's x/y bytes");
    check(t, (dos_uchar)g2f30[7] == 11 && (dos_uchar)g2f30[8] == 21, "record 1's x/y bytes");
    check(t, (dos_uchar)g2f30[12] == 12 && (dos_uchar)g2f30[13] == 22, "record 2's x/y bytes");
}

/* The ASM computes the new record's offset as an 8-BIT multiply
 * (count*5 truncated to a byte) before zero-extending into the DI add --
 * see asm_drawqbuf.c's comment.  Not reachable through draw_queue_append's
 * own increment (g2f30 is only 32 records deep), but exercised here
 * directly by pre-seeding the count byte, matching this port's brief to
 * preserve 8/16-bit wraparound the ASM relied on. */
static void test_append_count_byte_wraparound(void)
{
    const char *t = "append_count_byte_wraparound";
    memset(g2f30, 0xAA, sizeof g2f30);
    g2f30[0] = (dos_uchar)53; /* 53*5 = 265; (uint8_t)265 == 9 */

    draw_queue_append(0x42, 1, 2, 3, 4);

    check(t, g2f30[0] == 54, "count byte must still increment normally (53 -> 54)");
    check(t, (dos_uchar)g2f30[1 + 9] == 0x42, "record offset must use the 8-bit-truncated count*5 (9), landing at g2f30[10]");
}

int main(void)
{
    test_reset_zeroes_count();
    test_append_basic_packing();
    test_append_sequential_records();
    test_append_count_byte_wraparound();

    if (g_failures) {
        fprintf(stderr, "test_asm_drawqbuf: %d failure(s)\n", g_failures);
        return 1;
    }
    printf("test_asm_drawqbuf: OK\n");
    return 0;
}
