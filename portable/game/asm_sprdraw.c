/* asm_sprdraw.c -- semantic port of asm/SPRDRAW.ASM (M_6036_6181):
 * sprite_table_queue_draws (F_6036), animated_tile_tick (F_60A9),
 * sprite_record_adjust_draw (F_6181).
 *
 * See portable/include/asm_sprites.h for the shared record layout and the
 * storage-identity findings this file's own port work depends on:
 *   - `objtab` and the ASM's `_g40d0` extern are the SAME object (DS:40D0).
 *     animated_tile_tick therefore walks the identical table
 *     sprite_table_queue_draws/sprite_record_adjust_draw walk.
 *   - `sprite_tile_bank` and the ASM's `_sprbase` extern are the SAME
 *     object (DS:BFC8).
 *   - `g0a20` (DS:0A20) IS `_ga20` -- the SAME 2-byte word.
 *     sprite_table_queue_draws sets it to 10 once it drains objtab;
 *     animated_tile_tick decrements that SAME word every call and only
 *     does its own per-tick work once it reaches 0, then resets it to 10
 *     again.  The two routines therefore interlock through this one
 *     counter (animated_tile_tick's "every 10th call" throttle really
 *     means "every 10th call since objtab was last fully queued").  It is
 *     declared `uint8_t g0a20[2]` (not a scalar dos_uint) by the
 *     generator; g0a20_get()/g0a20_set() below read/write it as the
 *     little-endian 16-bit word the ASM's `mov word ptr`/`dec`/`cmp`
 *     instructions treat it as.
 *
 * IMPORTANT polarity finding for the port report: animated_tile_tick only
 * proceeds past its raycast_trail_active gate when that flag is ZERO
 * ("cmp _raycast_trail_active,0; je at_armed" -- je on equal-to-zero),
 * i.e. the OPPOSITE polarity from sprite_script_frame_driver's
 * check_bounds gate on the very same DS:08FE object (asm_sprites.c),
 * which proceeds only when it is NONZERO.  Both are transcribed literally.
 */
#include "game.h"
#include "asm_sprites.h"

static dos_uint g0a20_get(void)
{
    return (dos_uint)(g0a20[0] | (g0a20[1] << 8));
}

static void g0a20_set(dos_uint v)
{
    g0a20[0] = (uint8_t)(v & 0xFFu);
    g0a20[1] = (uint8_t)((v >> 8) & 0xFFu);
}

/* Advance one sprite_anim_record's 5-bit frame field by one step, wrapping
 * at the two ends (0 and SPRITE_ANIM_FRAME_MAX == 0x17, NOT 0x1F), per the
 * direction bit (SPRITE_ANIM_FLAG_DIR_DOWN).  Shared body of
 * animated_tile_tick's at_store and sprite_record_adjust_draw's identical
 * inline logic (both ASM bodies are instruction-for-instruction the same). */
static dos_uchar sprite_anim_advance_frame(struct sprite_anim_record *r)
{
    dos_uchar flags = r->flags;
    dos_uchar frame = (dos_uchar)(flags & SPRITE_ANIM_FRAME_MASK);

    if (flags & SPRITE_ANIM_FLAG_DIR_DOWN) {
        if (frame == 0) frame = (dos_uchar)SPRITE_ANIM_FRAME_MAX;
        else             frame--;
    } else {
        if (frame >= SPRITE_ANIM_FRAME_MAX) frame = 0;
        else                                 frame++;
    }
    r->flags = (dos_uchar)((flags & ~SPRITE_ANIM_FRAME_MASK) | frame);
    return frame;
}

/* ---- F_6036: sprite_table_queue_draws --------------------------------
 * Walk objtab (count-prefixed, 3-byte records) and, for every record,
 * fetch/validate its sprite frame via draw_queue_append(slot,x,y,0x0F,
 * 0x1E) (slot starts at 0x30 and increments per record) then append a
 * compact draw record via gfx_copy_rect(x*2, y+0xB8, frame_ptr, 0). Leaves
 * g0a20 = 10 once the table is exhausted.
 */
void sprite_table_queue_draws(void)
{
    dos_uchar count = (dos_uchar)objtab[0];
    struct sprite_anim_record *records = (struct sprite_anim_record *)(objtab + 1);
    dos_uint slot = 0x2F;
    dos_uint i;

    for (i = 0; i < count; i++) {
        struct sprite_anim_record *r = &records[i];
        dos_uchar frame = (dos_uchar)(r->flags & SPRITE_ANIM_FRAME_MASK);
        const dos_char *frame_ptr = sprite_tile_bank + (dos_uint)(frame * SPRITE_FRAME_STRIDE + SPRITE_FRAME_HEADER);
        dos_int x = (dos_int)(dos_uchar)r->x;   /* zero-extended, NOT doubled for draw_queue_append */
        dos_int y = (dos_int)(dos_uchar)r->y;

        slot++; /* inc bp, before use -- first record uses slot 0x30 */
        draw_queue_append((dos_char)slot, x, y, 0x0F, 0x1E);
        gfx_copy_rect((dos_int)((dos_uint)x << 1), (dos_int)(y + 0xB8), (const uint8_t *)frame_ptr, 0);
    }

    g0a20_set(10);
}

/* ---- F_60A9: animated_tile_tick ---------------------------------------
 * Every 10th call (shared g0a20/ga20 countdown, see header note) walk
 * objtab/g40d0 and, for every ARMED record (flag bit 7), advance its
 * frame, then repaint it: restore the background at both the primary row
 * and the 0xB8-mirror row from a clean backup 0x148 rows below, draw the
 * new frame, then copy the freshly-drawn pixels into the mirror row.
 */
void animated_tile_tick(void)
{
    dos_uint counter;
    dos_uchar count;
    struct sprite_anim_record *records;
    dos_uint i;

    if (raycast_trail_active != 0) return; /* at_done -- see header note on the polarity */

    counter = g0a20_get();
    counter = (dos_uint)(counter - 1);
    g0a20_set(counter);
    if (counter != 0) return; /* at_done */

    g0a20_set(0x0A);

    count = (dos_uchar)objtab[0];
    records = (struct sprite_anim_record *)(objtab + 1);

    for (i = 0; i < count; i++) {
        struct sprite_anim_record *r = &records[i];
        dos_uchar frame;
        const dos_char *frame_ptr;
        dos_int x, y;

        if ((r->flags & SPRITE_ANIM_FLAG_ARMED) == 0) continue; /* at_next */

        frame = sprite_anim_advance_frame(r);
        frame_ptr = sprite_tile_bank + (dos_uint)(frame * SPRITE_FRAME_STRIDE + SPRITE_FRAME_HEADER);
        x = (dos_int)((dos_uint)r->x << 1); /* shl bx,1 -- full 16-bit shift, not re-truncated to a byte */
        y = (dos_int)(dos_uchar)r->y;

        /* animated_tile_tick patches an already-pushed gfx_wipe_rect
         * argument (dy) between the first and second call rather than
         * re-pushing all six -- ported as two explicit calls with the
         * same source rect (x, y+0x148) and destinations y then y+0xB8. */
        gfx_wipe_rect(x, (dos_int)(y + 0x148), 0x1E, 0x1E, x, y);
        gfx_wipe_rect(x, (dos_int)(y + 0x148), 0x1E, 0x1E, x, (dos_int)(y + 0xB8));

        gfx_copy_rect(x, y, (const uint8_t *)frame_ptr, 0);

        gfx_wipe_rect(x, y, 0x1E, 0x1E, x, (dos_int)(y + 0xB8));
    }
}

/* ---- F_6181: sprite_record_adjust_draw -------------------------------
 * Advance objtab record `id`'s frame (same wraparound as
 * animated_tile_tick, but with NO "armed" bit test) and issue the same
 * wipe/copy/wipe redraw sequence at its (x*2, y).
 */
void sprite_record_adjust_draw(dos_int id)
{
    struct sprite_anim_record *records = (struct sprite_anim_record *)(objtab + 1);
    struct sprite_anim_record *r = &records[(dos_uint)id];
    dos_uchar frame = sprite_anim_advance_frame(r);
    const dos_char *frame_ptr = sprite_tile_bank + (dos_uint)(frame * SPRITE_FRAME_STRIDE + SPRITE_FRAME_HEADER);
    dos_int x = (dos_int)((dos_uint)r->x << 1);
    dos_int y = (dos_int)(dos_uchar)r->y;

    gfx_wipe_rect(x, (dos_int)(y + 0x148), 0x1E, 0x1E, x, y);
    gfx_wipe_rect(x, (dos_int)(y + 0x148), 0x1E, 0x1E, x, (dos_int)(y + 0xB8));
    gfx_copy_rect(x, y, (const uint8_t *)frame_ptr, 0);
    gfx_wipe_rect(x, y, 0x1E, 0x1E, x, (dos_int)(y + 0xB8));
}
