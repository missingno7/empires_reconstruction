/* asm_animrow.h -- row-copy "dissolve" tables, asm/ANIMROW.ASM
 * (`_anim_step_row_copy`, F_9EC3).
 *
 * DATA_10EE0 (DS:12B0, 32 bytes, portable/generated/game_data.c) is two
 * back-to-back 16-entry byte tables the ASM reaches with `xlat`:
 *
 *   - DS:[12B0h] == DATA_10EE0[0..15]:  "seed" table, indexed by the
 *     animation step `i` (0..15 -- anim_step_loop's own 0x10-iteration
 *     counter, src/ANIMSTEP.C) -> the within-16-byte-block offset
 *     revealed on the first row of that step.
 *   - DS:[12C0h] == DATA_10EE0[16..31]: "chain" table, indexed by the
 *     PREVIOUS block offset -> the block offset for the next row.
 *     Applied once per row, including row 0 (so row 0's offset is
 *     chain[seed[i]], not seed[i] itself -- see asm_animrow.c).
 *
 * Both halves are permutations of 0..15 (see the literal values in
 * game_data.c), so chaining through the second table visits all 16
 * within-block byte positions once every 16 rows.  Per row, the routine
 * copies exactly ONE byte out of every 16-byte block of the row (at the
 * position this chain lands on) and leaves the rest of the block alone;
 * across the 16 animation steps (i=0..15) and enough rows, every byte of
 * every block eventually gets copied.  This is the "venetian blind" /
 * dissolve reveal asm-module-inventory.md sec 5 flags for src/ANIMSTEP.C's
 * horizontal scroll transition: rather than a plain left-to-right wipe,
 * the chain table scatters which byte of each 16-byte block is revealed
 * from one scanline to the next.
 *
 * The ASM's `xlat` performs no bounds masking -- it trusts AL to already
 * be in range.  That is true here only because every caller keeps the
 * seed index and the chain values within 0..15 (the fixed 16-step
 * animation loop, and the tables' own closure under permutation); the
 * accessors below mask defensively so out-of-range input cannot read
 * outside DATA_10EE0 in C (undefined behaviour that plain `xlat` would
 * not have hit in the historical flat DOS address space).
 */
#ifndef PORTABLE_ASM_ANIMROW_H
#define PORTABLE_ASM_ANIMROW_H

#include "dos_types.h"
#include "game_data.h" /* DATA_10EE0[32] */

#define ANIMROW_XLAT_BLOCK_BYTES 0x10u /* 16: block size the tables step over */

static inline dos_uchar animrow_xlat_seed(dos_uchar step_index)
{
    return DATA_10EE0[step_index & 0x0Fu];
}

static inline dos_uchar animrow_xlat_chain(dos_uchar prev)
{
    return DATA_10EE0[0x10u + (prev & 0x0Fu)];
}

#endif /* PORTABLE_ASM_ANIMROW_H */
