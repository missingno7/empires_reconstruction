/* asm_drawq.c -- semantic port of asm/DRAWQ.ASM:
 * _draw_queue_render_highlighted (F_D61C), _draw_queue_render (F_D79C).
 *
 * See asm_drawq.h for: the corrected identity of DS:0BFC0h
 * (`record_table_root`, NOT g2f30 -- asm-module-inventory.md's guess was
 * wrong, disproved by the 4-byte vs 5-byte record-size mismatch), the
 * record layout, the g2380 colour-slot/chunk shape, and why the in-place
 * record mutation is real (not dead code) and forces a mutable `list`.
 *
 * Both game_funcs.h entry points take no arguments (matching every real
 * C caller: src/BOARD.C's `draw_queue_render_highlighted()`, src/GAME.C's
 * `draw_queue_render()`), exactly as the ASM itself reads its list
 * through a fixed DGROUP cell rather than a passed-in register/stack
 * argument -- there is no ES:DI-passing C wrapper anywhere in src/*.C for
 * either routine.  `draw_queue_render_list()` (asm_drawq.h) is the
 * explicit-pointer core the brief for this module asked for; the two
 * public zero-arg entry points are thin shims over it, plugging in
 * `record_table_root` the same way the ASM's own `les di,ds:[0bfc0h]`
 * did.
 */
#include "game.h"
#include "asm_drawq.h"

/* g2380 is generated as `dos_char g2380[23][130]` (matching src/BOARD.C's
 * own `extern char g2380[][0x82];` declaration) so that BOARD.C's future
 * port can keep indexing it row-major.  DRAWQ.ASM itself addresses the
 * SAME memory as one flat run of bytes (its `mul dx`-computed index is a
 * raw DS-relative displacement, not a row/column pair), so every access
 * below goes through this flat byte view instead of `g2380[row]`. */
static dos_char *const drawq_g2380_flat = &g2380[0][0];

/* The 24 fixed g2380 byte offsets asm_drawq.h's highlighted-only "shared
 * highlight pixel" section describes: +0x82,+0x62,+0x92 repeating from
 * 0x21, i.e. one byte inside each of the 8 colour slots' 3 sub-images. */
static const dos_uint DRAWQ_HIGHLIGHT_OFFSETS[24] = {
    0x021u, 0x0a3u, 0x105u, 0x197u, 0x219u, 0x27bu, 0x30du, 0x38fu,
    0x3f1u, 0x483u, 0x505u, 0x567u, 0x5f9u, 0x67bu, 0x6ddu, 0x76fu,
    0x7f1u, 0x853u, 0x8e5u, 0x967u, 0x9c9u, 0xa5bu, 0xaddu, 0xb3fu
};

static void drawq_set_highlight_bytes(dos_uchar value)
{
    size_t i;
    for (i = 0; i < 24; i++)
        drawq_g2380_flat[DRAWQ_HIGHLIGHT_OFFSETS[i]] = (dos_char)value;
}

/* IMPORTANT semantic hazard, faithfully preserved from the ASM (same class
 * as asm_rectq.c/asm_boardcol.c): the record loop has NO zero-count guard
 * -- `sub ch,ch; mov cl,es:[di]; inc di` falls straight into the loop
 * body, and the closing x86 `loop` only stops AFTER at least one pass, so
 * `list[0] == 0` would process one bogus record and then wrap cx to
 * 0xFFFF and keep going far out of bounds.  Both real historical callers
 * guard against this themselves (`if (*record_table_root != 0)
 * draw_queue_render[...]();`, src/BOARD.C and src/GAME.C) -- callers of
 * this function, including tests, MUST NOT pass a `list` whose first byte
 * is 0. */
void draw_queue_render_list(uint8_t *list, dos_int highlighted)
{
    dos_uint count;
    uint8_t *p;

    if (list == NULL)
        return; /* defensive; the ASM would fault on a null far pointer */

    if (highlighted) {
        g96 = 0x190;                      /* mov word ptr ds:[96h],190h */
        drawq_set_highlight_bytes(0x10u); /* 24x set_cell_attribute ...,10h */
    }

    count = list[0];   /* mov cl,es:[di] (ch already 0) */
    p = list + 1;

    do {
        dos_uchar attr       = p[0];
        dos_uchar x_lo        = p[1];
        dos_uchar strip_count = p[2];
        dos_uchar color_byte  = p[3];
        dos_int strip;
        dos_int x, y;
        dos_int color_ext;
        dos_uint off_left, off_mid, off_right;
        dos_uchar new_color;

        p += DRAW_QUEUE_RENDER_RECORD_BYTES; /* di advances 4 bytes/record (see asm_drawq.h) */

        /* Real, observable write-back (not dead code -- see asm_drawq.h):
         * cycles record.color through 4 values across successive frames. */
        new_color = ((color_byte & 3u) == 3u) ? (dos_uchar)(color_byte - 3u)
                                               : (dos_uchar)(color_byte + 1u);
        p[-1] = new_color; /* the record's byte3, i.e. p[3] before the advance above */

        /* record.color's real value range is not verified: it comes from
         * `board_records`-resident level/map data (see asm_drawq.h), which
         * this port did not decode.  8 g2380 colour slots (0..7) is the
         * natural fit (resource_scoreboard_unpack's own loop runs exactly
         * 8 times, each writing one 0x176-byte slot) but 8*0x176 = 2992
         * bytes while the generator sized g2380 at 2990 (23*130) -- i.e.
         * even a well-formed color==7 reaches 2 bytes past the modelled
         * array (resource_scoreboard_unpack's own last memmove would
         * write those same 2 bytes historically; this is a src/BOARD.C
         * port question, flagged here because DRAWQ.ASM's read side hits
         * it too).  A color byte of 8+ or negative walks further off the
         * end, same as the ASM itself would (`les`-relative, no bounds
         * check).  Flagged as an open question in the port report, not
         * fixed here. */
        color_ext = (dos_int)(int8_t)color_byte;                        /* cbw */
        off_left  = (dos_uint)dos_mul16(color_ext, (dos_int)DRAW_QUEUE_RENDER_SLOT_BYTES); /* mul dx; add ax,2380h folds away against g2380's own base -- see asm_drawq.h */
        off_mid   = dos_uadd16(off_left, DRAW_QUEUE_RENDER_LEFT_CAP_BYTES);
        off_right = dos_uadd16(off_mid, DRAW_QUEUE_RENDER_MIDDLE_TILE_BYTES);

        x = (dos_int)(2u * (dos_uint)attr); /* mov al,bl; shl ax,1 (bl=attr, ah cleared beforehand) */
        y = (dos_int)(dos_uint)x_lo;        /* mov ah,dl(=0); mov al,bh(=x_lo) */
        if (highlighted)
            y = dos_add16(y, 0xb8); /* highlighted-only: add ax,0b8h (see asm_drawq.h) */

        gfx_copy_rect(x, y, (const uint8_t *)(drawq_g2380_flat + off_left), 0);

        x = dos_add16(x, 0x0c);
        for (strip = 0; strip < (dos_int)strip_count; strip++) {
            gfx_blit_bitmap(x, y, (const uint8_t *)(drawq_g2380_flat + off_mid));
            x = dos_add16(x, 8);
        }

        gfx_copy_rect(x, y, (const uint8_t *)(drawq_g2380_flat + off_right), 0);

        count = (dos_uint)(count - 1); /* loop D61_record_loop / D79_record_loop (post-decrement, wraps on 0) */
    } while (count != 0);

    if (highlighted) {
        drawq_set_highlight_bytes(0x0bu); /* 24x set_cell_attribute ...,0bh */
        g96 = 0x9f;                       /* mov word ptr ds:[96h],9fh */
    }
}

void draw_queue_render(void)
{
    draw_queue_render_list((uint8_t *)record_table_root, 0);
}

void draw_queue_render_highlighted(void)
{
    draw_queue_render_list((uint8_t *)record_table_root, 1);
}
