/* test_asm_animrow.c -- unit tests for portable/game/asm_animrow.c
 * (anim_step_row_copy, the port of asm/ANIMROW.ASM's _anim_step_row_copy).
 *
 * Both cases below are hand-traced against DATA_10EE0's actual literal
 * bytes (portable/generated/game_data.c) -- NOT derived by calling the
 * production seed/chain accessors -- so a correct-looking-but-wrong port
 * (e.g. a table indexing slip) would still be caught.  DATA_10EE0[0..15]
 * (the "seed" table) is {13,11,6,10,3,1,15,0,12,5,7,2,4,14,9,8};
 * DATA_10EE0[16..31] (the "chain" table) is
 * {11,13,5,14,7,1,15,2,0,3,8,4,9,12,6,10}.
 */
#include "game.h"
#include "asm_animrow.h"

#include <stdio.h>
#include <string.h>

static int g_failures = 0;

static void fail(const char *test, const char *what)
{
    fprintf(stderr, "test_asm_animrow: FAIL %s: %s\n", test, what);
    g_failures++;
}

static void check(const char *test, int cond, const char *what)
{
    if (!cond) fail(test, what);
}

/* Fills g3924[row][0..n) with value = base + offset, so every touched
 * byte's value identifies exactly which offset it came from. */
static void fill_row(dos_int row, uint8_t base, int n)
{
    for (int i = 0; i < n; i++) g3924[row][i] = (uint8_t)(base + i);
}

/* Single block (width == ANIMROW_XLAT_BLOCK_BYTES), 3 rows, step_index=0.
 * Hand trace (seed[0]=13, chain[13]=12, chain[12]=9, chain[9]=3):
 *   row1: seg=chain[13]=12 -> touches relative offset 12
 *   row2: seg=chain[12]=9  -> touches relative offset 9
 *   row3: seg=chain[9]=3   -> touches relative offset 3
 * row_stride=20, width=16 -> row_skip=4, so successive row spans start at
 * flat offsets 0, 20, 40 -> touched absolute offsets 12, 29, 43. */
static void test_single_block_three_rows(void)
{
    const char *t = "single_block_three_rows";
    const int span = 60; /* 3 rows * 20 */

    fill_row(0, 0x40, span);   /* source: g3924[0][i] = 0x40+i */
    memset(g3924[10], 0xEE, (size_t)span); /* dest sentinel */

    anim_step_row_copy(/*src_off=*/0, /*src_row_idx=*/0, /*width=*/16,
                        /*rows=*/3, /*dst_off=*/0, /*dst_row_idx=*/10,
                        /*step_index=*/0, /*row_stride=*/20);

    static const int touched[3] = {12, 29, 43};
    for (int i = 0; i < span; i++) {
        int is_touched = (i == touched[0] || i == touched[1] || i == touched[2]);
        uint8_t expect = is_touched ? (uint8_t)(0x40 + i) : 0xEE;
        if (g3924[10][i] != expect) {
            fprintf(stderr, "test_asm_animrow: FAIL %s: byte %d = 0x%02x, expected 0x%02x\n",
                    t, i, g3924[10][i], expect);
            g_failures++;
        }
    }
    /* Source row must be untouched (copy direction is src -> dst only). */
    check(t, g3924[0][12] == 0x40 + 12, "source row must not be modified");
}

/* Three blocks (width=48), one row, row_stride==width (row_skip=0),
 * step_index=5.  Hand trace: seed[5]=1, chain[1]=13; block loop touches
 * relative offsets 13, 16+13=29, 32+13=45 before remaining(=48-48=0)
 * stops the loop (0 is not > 13). */
static void test_multi_block_one_row(void)
{
    const char *t = "multi_block_one_row";
    const int span = 48;

    fill_row(1, 0x80, span);
    memset(g3924[11], 0x11, (size_t)span);

    anim_step_row_copy(0, 1, 48, 1, 0, 11, 5, 48);

    static const int touched[3] = {13, 29, 45};
    for (int i = 0; i < span; i++) {
        int is_touched = (i == touched[0] || i == touched[1] || i == touched[2]);
        uint8_t expect = is_touched ? (uint8_t)(0x80 + i) : 0x11;
        if (g3924[11][i] != expect) {
            fprintf(stderr, "test_asm_animrow: FAIL %s: byte %d = 0x%02x, expected 0x%02x\n",
                    t, i, g3924[11][i], expect);
            g_failures++;
        }
    }
}

/* The seed/chain accessors themselves, checked against the literal table
 * values (belt-and-suspenders on top of the two end-to-end cases above). */
static void test_xlat_accessors(void)
{
    const char *t = "xlat_accessors";
    check(t, animrow_xlat_seed(0) == 13, "seed[0]");
    check(t, animrow_xlat_seed(7) == 0, "seed[7]");
    check(t, animrow_xlat_seed(15) == 8, "seed[15]");
    check(t, animrow_xlat_chain(0) == 11, "chain[0]");
    check(t, animrow_xlat_chain(13) == 12, "chain[13]");
    check(t, animrow_xlat_chain(9) == 3, "chain[9]");
}

int main(void)
{
    display_mode = 5;
    gfx_framebuffer_init(); /* real, contiguous g3924 rows to copy through */

    test_xlat_accessors();
    test_single_block_three_rows();
    test_multi_block_one_row();

    gfx_framebuffer_shutdown();

    if (g_failures) {
        fprintf(stderr, "test_asm_animrow: %d failure(s)\n", g_failures);
        return 1;
    }
    printf("test_asm_animrow: OK\n");
    return 0;
}
