/* asm_sprites.c -- semantic port of asm/SPRITES.ASM (M_4AA8_4EEB):
 * play_window_wipe_clipped (F_4AA8), sprite_script_frame_driver (F_4B0C),
 * sprite_table_wipe_active (F_4E9F), board_actors_draw (F_4EEB).
 *
 * See portable/include/asm_sprites.h for the shared record layout, the
 * storage-identity findings (objtab==g40d0, sprbase==sprite_tile_bank,
 * board_record_index, actor_record_table/actor_state_table) and the
 * bytecode opcode numbering caveat.
 *
 * None of the four routines is a Turbo C translation unit (asm/SPRITES.ASM's
 * own header comments say so for each one individually); they are ported
 * here as ordinary typed C functions with identical observable behaviour
 * (gfx_* call order/arguments, DGROUP reads/writes, 16-bit truncation)
 * rather than instruction-for-instruction.
 */
#include "game.h"
#include "trace.h"
#include "asm_sprites.h"

#ifdef _MSC_VER
/* Several sites below take the address of a dos_uint field inside
 * #pragma pack(1) struct actor_record (e.g. &rec->timer_11) to pass into
 * shared helper functions; the resulting pointer can be odd-aligned,
 * which is exactly what the ASM's own byte-granular DI-relative
 * addressing does too (x86/x64 handle unaligned 16-bit access natively).
 */
#pragma warning(disable : 4366)
#endif

static const uint8_t *actor_record_bitmap(const struct actor_record *rec)
{
    return (const uint8_t *)resource_ptr_table[rec->sprite_frame];
}

/* ---- F_4AA8: play_window_wipe_clipped -------------------------------
 * Clip (x1,y1,w,h) to the play window (8..0x137, 0x10..0x9F); on success
 * call gfx_wipe_rect with the clipped rect and a source row 0xB8 below the
 * destination (the "mirror" backup copy convention used throughout the
 * VGA driver -- see architecture.md and animated_tile_tick's identical
 * y+0xB8 mirroring below).  No C caller; called only from
 * sprite_table_wipe_active (ASM-to-ASM).
 */
void play_window_wipe_clipped(dos_int x1, dos_int y1, dos_int w, dos_int h)
{
    dos_int x2, y2;

    if (x1 > 0x137) return;             /* cmp ax,137h; jg pw_out */
    if (y1 > 0x9F)  return;             /* cmp bx,9Fh; jg pw_out */

    x2 = x1 + w - 1;                    /* add cx,ax; dec cx */
    if (x2 < 8) return;                 /* cmp cx,8; jl pw_out */
    y2 = y1 + h - 1;                    /* add dx,bx; dec dx */
    if (y2 < 0x10) return;              /* cmp dx,10h; jl pw_out */

    if (x1 < 8)      x1 = 8;
    if (x2 > 0x137)  x2 = 0x137;
    if (y1 < 0x10)   y1 = 0x10;
    if (y2 > 0x9F)   y2 = 0x9F;

    w = x2 - x1 + 1;
    h = y2 - y1 + 1;

    gfx_wipe_rect(x1, y1 + 0xB8, w, h, x1, y1);
}

/* ---- F_4E9F: sprite_table_wipe_active --------------------------------
 * Walk actor_record_table/actor_state_table (count-prefixed, 0x20-byte
 * records) and call play_window_wipe_clipped for every record belonging
 * to the current board (board_id == board_record_index) whose `active`
 * byte is not 1.
 */
void sprite_table_wipe_active(void)
{
    dos_uint count = actor_record_table[0];
    dos_uint i;

    for (i = 0; i < count; i++) {
        struct actor_record *rec = (struct actor_record *)&actor_state_table[i];

        if ((dos_int)rec->board_id != board_record_index) continue; /* wa_next */
        if (rec->active == 1) continue;                              /* wa_next */

        play_window_wipe_clipped(rec->x, rec->y,
                                  tile_width_table[rec->sprite_frame],
                                  tile_height_table[rec->sprite_frame]);
    }
}

/* ---- F_4EEB: board_actors_draw ---------------------------------------
 * Same table walk as sprite_table_wipe_active, but blits each record's
 * sprite frame (color 0x0F selected once up front) at (x, y0+y), and,
 * when vline_extra is nonzero, an extra gfx_vline strip.
 */
void board_actors_draw(dos_int y0)
{
    dos_uint count = actor_record_table[0];
    dos_uint i;

    gfx_color_select(0x0F);

    for (i = 0; i < count; i++) {
        struct actor_record *rec = (struct actor_record *)&actor_state_table[i];

        if ((dos_int)rec->board_id != board_record_index) continue; /* skip_record */
        if (rec->active == 1) continue;                              /* skip_record */

        if (!actor_record_bitmap(rec))
            EMPIRES_TRACE("board_actors_draw: record %u frame %u has no bitmap (board %d)",
                          (unsigned)i, (unsigned)rec->sprite_frame, (int)board_record_index);
        gfx_copy_rect(rec->x, (dos_int)(y0 + rec->y),
                      actor_record_bitmap(rec), rec->dir_flip);

        if (rec->vline_extra != 0) {
            /* mov ax,[di+2h]; add ax,10h / mov bx,[di+4h]; sub bx,dx; inc bx
             * -- transcribed literally, including the apparently-swapped
             * y/n roles (gfx_vline's 2nd arg gets vline_extra, not a row;
             * its 3rd arg gets a row-shaped value, not a count).  Identical
             * instruction sequence appears in sprite_script_frame_driver's
             * update_record below ("mirrors F_4EEB's copy step" per the
             * ASM's own comment); not "fixed" here, see the port report. */
            gfx_vline((dos_int)(rec->x + 0x10), (dos_int)rec->vline_extra,
                      (dos_int)(rec->y - rec->vline_extra + 1));
        }
    }
}

/* ---- F_4B0C: sprite_script_frame_driver ------------------------------
 * The per-tick bytecode interpreter.  See asm_sprites.h for the opcode
 * numbering caveat (UNVERIFIED against the real DS:074A jump table) and
 * the SPRITE_SCRIPT_BASE/saved_pc storage-location caveat.
 *
 * Structured with goto to mirror the ASM's own label graph directly
 * (record_loop / dispatch_opcode / render_record / update_record /
 * collision_check / skip_collision / check_special_flags / reset_fields /
 * check_bounds) rather than restructuring it -- for a routine this
 * control-flow-heavy, a 1:1 label mapping is the lowest-risk translation.
 */

/* Shared body of opcode_skip_bytes (0x4e84): read one operand byte, look
 * it up in the skip-length table at DS:0782 (b740[0x0782-0x0740]),
 * advance si by that many bytes.  NOT a dispatch-table entry itself (see
 * asm_sprites.h's enum comment) -- reached only via direct jumps from the
 * "-> skip" opcodes below (19..26) and via fall-through from
 * op_random_skip (27). b740 is declared dos_uchar[278] (DS:0740..DS:0855);
 * the length-table index is an unbounded bytecode-supplied byte (0..255)
 * with no historical bounds check, so an index above 0x115 (0x740+0x782-
 * 0x740+0xFF exceeds the 278-byte extent) can read past b740's modelled
 * extent -- flagged in the port report, not defended against here
 * (matches the ASM exactly). */
static void sprite_script_skip_bytes(dos_uint *si)
{
    dos_uchar idx = SPRITE_SCRIPT_BASE[*si]; *si = (dos_uint)(*si + 1); /* lodsb */
    dos_uchar len = b740[0x0782 - 0x0740 + idx];                       /* mov bl,[bx+782h] */
    *si = (dos_uint)(*si + len);                                        /* add si,bx */
}

/* Shared body of the four class-flag-bit test opcodes (19..22): word
 * operand indexes b4374 (DS:4374, only 1 byte modelled -- see the port
 * report), test against `mask`. */
static dos_uchar sprite_script_test_class_bit(dos_uint *si, dos_uchar mask)
{
    dos_int word_lo = SPRITE_SCRIPT_BASE[*si];
    dos_int word_hi = SPRITE_SCRIPT_BASE[(dos_uint)(*si + 1)];
    dos_uint operand = (dos_uint)(word_lo | (word_hi << 8));            /* lodsw */
    *si = (dos_uint)(*si + 2);
    return (dos_uchar)(b4374[operand] & mask);
}

/* Shared body of the four cursor-position test opcodes (23..26). */
static dos_uchar sprite_script_fetch_byte_operand(dos_uint *si)
{
    dos_uchar b = SPRITE_SCRIPT_BASE[*si];
    *si = (dos_uint)(*si + 1);
    return b;
}

/* Shared countdown-timer opcode body (4..6): reload/skip-length words,
 * decrement *timer, reload+skip on underflow, skip while still running,
 * fall through (no skip) exactly when the decrement hits zero. */
static void sprite_script_countdown(dos_uint *si, dos_uint *timer)
{
    dos_int skip_len = (dos_int)(dos_uint)(SPRITE_SCRIPT_BASE[*si] | (SPRITE_SCRIPT_BASE[(dos_uint)(*si + 1)] << 8));
    dos_int reload_val;
    dos_uint newval;
    *si = (dos_uint)(*si + 2);
    reload_val = (dos_int)(dos_uint)(SPRITE_SCRIPT_BASE[*si] | (SPRITE_SCRIPT_BASE[(dos_uint)(*si + 1)] << 8));
    *si = (dos_uint)(*si + 2);

    newval = (dos_uint)(*timer - 1);
    *timer = newval;
    if (newval == 0) {
        /* zf: expired exactly now -- run the guarded block, no skip */
    } else if ((dos_int)newval > 0) {
        *si = (dos_uint)(*si + skip_len);
    } else {
        *timer = (dos_uint)reload_val;
        *si = (dos_uint)(*si + skip_len);
    }
}

/* ---- opcode handler table --------------------------------------------
 * Ties each of the 28 opcodes to the real historical dispatch table,
 * `DATA_01037A_POINTER_TABLE` (src/data/DATA_01037A_POINTER_TABLE.json,
 * DS:074A, 28 x u16 code offsets inside this module) -- see the mapping
 * from table index to ASM label in asm_sprites.h's enum comment, derived
 * by disassembling asm/SPRITES.ASM's F_4B0C member
 * (build/F_4B0C.ndisasm.txt) and matching each table offset to a real
 * instruction address.  The generator will eventually emit that JSON as
 * `void (*DATA_01037A_POINTER_TABLE[28])(void)`; sprite_script_dispatch[]
 * below is this module's own copy, in the same order, until then.
 *
 * A handler returns SS_CONTINUE to resume dispatch at the next opcode
 * (`goto dispatch_opcode` in the ASM), SS_YIELD_RENDER to fall into
 * render_record without a board-id recheck (set_program_counter only), or
 * SS_YIELD_NEXT to run the ASM's shared save_program_counter tail (saved
 * already by the handler; the caller performs the one board-id recheck
 * and either resumes at update_record or moves to the next record).
 */
enum sprite_script_control { SS_CONTINUE, SS_YIELD_RENDER, SS_YIELD_NEXT };

typedef enum sprite_script_control (*sprite_script_handler_fn)(struct actor_record *rec, dos_uint *si, dos_uint idx);

static enum sprite_script_control op_set_pc(struct actor_record *rec, dos_uint *si, dos_uint idx)
{
    (void)idx;
    rec->saved_pc = *si; /* sub si,0b3aeh; mov[di+0dh],si */
    return SS_YIELD_RENDER;
}

static enum sprite_script_control op_add_offset(struct actor_record *rec, dos_uint *si, dos_uint idx)
{
    dos_int rel = (dos_int)(dos_uint)(SPRITE_SCRIPT_BASE[*si] | (SPRITE_SCRIPT_BASE[(dos_uint)(*si + 1)] << 8));
    (void)rec; (void)idx;
    *si = (dos_uint)(*si + 2 + rel);
    return SS_CONTINUE;
}

static enum sprite_script_control op_call(struct actor_record *rec, dos_uint *si, dos_uint idx)
{
    dos_int rel = (dos_int)(dos_uint)(SPRITE_SCRIPT_BASE[*si] | (SPRITE_SCRIPT_BASE[(dos_uint)(*si + 1)] << 8));
    (void)idx;
    *si = (dos_uint)(*si + 2);
    rec->call_return_pc = *si;
    *si = (dos_uint)(*si + rel);
    return SS_CONTINUE;
}

static enum sprite_script_control op_return(struct actor_record *rec, dos_uint *si, dos_uint idx)
{
    (void)idx;
    *si = rec->call_return_pc;
    return SS_CONTINUE;
}

static enum sprite_script_control op_timer_11(struct actor_record *rec, dos_uint *si, dos_uint idx)
{ (void)idx; sprite_script_countdown(si, &rec->timer_11); return SS_CONTINUE; }
static enum sprite_script_control op_timer_13(struct actor_record *rec, dos_uint *si, dos_uint idx)
{ (void)idx; sprite_script_countdown(si, &rec->timer_13); return SS_CONTINUE; }
static enum sprite_script_control op_timer_15(struct actor_record *rec, dos_uint *si, dos_uint idx)
{ (void)idx; sprite_script_countdown(si, &rec->timer_15); return SS_CONTINUE; }

static enum sprite_script_control op_call_caf1(struct actor_record *rec, dos_uint *si, dos_uint idx)
{
    dos_uchar op1 = sprite_script_fetch_byte_operand(si);
    (void)rec; (void)idx;
    stream_control_block_arm((dos_int)op1);
    return SS_CONTINUE;
}

static enum sprite_script_control op_dispatch_2a2d(struct actor_record *rec, dos_uint *si, dos_uint idx)
{
    dos_uchar op1 = sprite_script_fetch_byte_operand(si);
    (void)rec; (void)idx;
    board_run_unit_script((dos_uchar *)record_field_skip_n((dos_int)op1));
    return SS_CONTINUE;
}

static enum sprite_script_control op_dispatch_36f0(struct actor_record *rec, dos_uint *si, dos_uint idx)
{
    dos_uchar op1 = sprite_script_fetch_byte_operand(si);
    (void)rec; (void)idx;
    board_advance_unit_moves((dos_int)op1);
    return SS_CONTINUE;
}

/* Shared body of opcode_set_flag's two entry points (dl=1 at 0x4d17,
 * dl=0 at 0x4d1b) -- operand >= 0x80: current-record-relative index
 * (idx + operand&0x7F); operand < 0x80: absolute actor_state_table index. */
static enum sprite_script_control sprite_script_set_flag(dos_uint *si, dos_uint idx, dos_uchar dl)
{
    dos_uchar operand = sprite_script_fetch_byte_operand(si);
    dos_uint target = (operand >= 0x80) ? (dos_uint)(idx + (operand & 0x7Fu)) : (dos_uint)operand;
    /* No historical bounds check either; actor_state_table has 12
     * elements -- an out-of-range target is UB here where the ASM would
     * just touch adjacent DGROUP bytes. Flagged in the port report. */
    ((struct actor_record *)&actor_state_table[target])->rendered = dl;
    return SS_CONTINUE;
}
static enum sprite_script_control op_set_flag1(struct actor_record *rec, dos_uint *si, dos_uint idx)
{ (void)rec; return sprite_script_set_flag(si, idx, 1u); }
static enum sprite_script_control op_set_flag0(struct actor_record *rec, dos_uint *si, dos_uint idx)
{ (void)rec; return sprite_script_set_flag(si, idx, 0u); }

static enum sprite_script_control op_set_limits(struct actor_record *rec, dos_uint *si, dos_uint idx)
{
    dos_int word_lo = SPRITE_SCRIPT_BASE[*si];
    dos_int word_hi = SPRITE_SCRIPT_BASE[(dos_uint)(*si + 1)];
    (void)idx;
    *si = (dos_uint)(*si + 2);
    rec->frame_limit_lo = (dos_uchar)word_lo;
    rec->frame_limit_hi = (dos_uchar)word_hi;
    return SS_CONTINUE;
}

static enum sprite_script_control op_set_velocity(struct actor_record *rec, dos_uint *si, dos_uint idx)
{
    dos_uchar operand = sprite_script_fetch_byte_operand(si);
    (void)idx;
    rec->sprite_frame = (dos_uchar)(operand & 0x7Fu);
    rec->dir_flip = (dos_uchar)((operand >> 7) & 1u);
    return SS_CONTINUE;
}

static enum sprite_script_control op_move_clamped(struct actor_record *rec, dos_uint *si, dos_uint idx)
{
    dos_char dx_delta = (dos_char)SPRITE_SCRIPT_BASE[*si];   /* cbw sign-extends this byte */
    dos_char dy_delta = (dos_char)SPRITE_SCRIPT_BASE[(dos_uint)(*si + 1)]; /* cbw sign-extends this byte */
    dos_uchar operand;
    dos_uchar candidate;
    (void)idx;
    *si = (dos_uint)(*si + 2);
    rec->x = (dos_int)(rec->x + dx_delta);
    rec->y = (dos_int)(rec->y + dy_delta);

    operand = sprite_script_fetch_byte_operand(si);
    rec->dir_flip = (dos_uchar)((operand >> 7) & 1u);
    candidate = (dos_uchar)((operand & 0x7Fu) + rec->sprite_frame);
    if (candidate < rec->frame_limit_lo)      candidate = rec->frame_limit_hi; /* clamp_high: wraps to the HIGH bound */
    else if (candidate > rec->frame_limit_hi) candidate = rec->frame_limit_lo; /* clamp_low: wraps to the LOW bound */

    /* velocity_store: */
    rec->sprite_frame = candidate;
    if (rec->active != 1) {
        gfx_copy_rect(rec->x, rec->y, actor_record_bitmap(rec), rec->dir_flip);
    }
    rec->saved_pc = *si;
    return SS_YIELD_NEXT;
}

static enum sprite_script_control op_set_position(struct actor_record *rec, dos_uint *si, dos_uint idx)
{
    dos_int word_lo = SPRITE_SCRIPT_BASE[*si];
    dos_int word_hi = SPRITE_SCRIPT_BASE[(dos_uint)(*si + 1)];
    dos_uchar operand;
    (void)idx;
    *si = (dos_uint)(*si + 2);
    rec->x = (dos_int)(word_lo * 2);
    rec->y = (dos_int)word_hi;

    operand = sprite_script_fetch_byte_operand(si);
    rec->sprite_frame = (dos_uchar)(operand & 0x7Fu);
    rec->dir_flip = (dos_uchar)((operand >> 7) & 1u);
    if (rec->active != 1) {
        gfx_copy_rect(rec->x, rec->y, actor_record_bitmap(rec), rec->dir_flip);
    }
    rec->saved_pc = *si;
    return SS_YIELD_NEXT;
}

static enum sprite_script_control op_set_position_and_frame(struct actor_record *rec, dos_uint *si, dos_uint idx)
{
    dos_int word_lo = SPRITE_SCRIPT_BASE[*si];
    dos_int word_hi = SPRITE_SCRIPT_BASE[(dos_uint)(*si + 1)];
    dos_int op2_lo, op2_hi;
    dos_uchar new_board;
    (void)idx;
    *si = (dos_uint)(*si + 2);
    rec->x = (dos_int)(word_lo * 2);
    rec->y = (dos_int)word_hi;

    op2_lo = SPRITE_SCRIPT_BASE[*si];
    op2_hi = SPRITE_SCRIPT_BASE[(dos_uint)(*si + 1)];
    *si = (dos_uint)(*si + 2);
    new_board = (dos_uchar)op2_hi;
    rec->board_id = new_board;
    rec->dir_flip = (dos_uchar)((op2_lo >> 7) & 1u);

    if (new_board == (dos_uchar)board_record_index) {
        rec->sprite_frame = (dos_uchar)(op2_lo & 0x7Fu);
        if (rec->active != 1) {
            gfx_copy_rect(rec->x, rec->y, actor_record_bitmap(rec), rec->dir_flip);
        }
    }
    /* else: move_no_redraw -- x/y/board_id/dir_flip already updated
     * above, but sprite_frame is left unchanged and no redraw happens. */
    rec->saved_pc = *si;
    return SS_YIELD_NEXT;
}

static enum sprite_script_control op_flag8_set(struct actor_record *rec, dos_uint *si, dos_uint idx)
{
    (void)idx;
    rec->active = 1;
    rec->special_handled = 0;
    rec->saved_pc = *si;
    return SS_YIELD_NEXT;
}

static enum sprite_script_control op_flag8_clear(struct actor_record *rec, dos_uint *si, dos_uint idx)
{
    (void)si; (void)idx;
    rec->active = 0;
    return SS_CONTINUE;
}

static enum sprite_script_control op_test_class_bits_7123_jz(struct actor_record *rec, dos_uint *si, dos_uint idx)
{ (void)rec; (void)idx; if (sprite_script_test_class_bit(si, 0x07) == 0) sprite_script_skip_bytes(si); return SS_CONTINUE; }
static enum sprite_script_control op_test_class_bits_7123_jnz(struct actor_record *rec, dos_uint *si, dos_uint idx)
{ (void)rec; (void)idx; if (sprite_script_test_class_bit(si, 0x07) != 0) sprite_script_skip_bytes(si); return SS_CONTINUE; }
static enum sprite_script_control op_test_class_bit_10_jnz(struct actor_record *rec, dos_uint *si, dos_uint idx)
{ (void)rec; (void)idx; if (sprite_script_test_class_bit(si, 0x10) != 0) sprite_script_skip_bytes(si); return SS_CONTINUE; }
static enum sprite_script_control op_test_class_bit_10_jz(struct actor_record *rec, dos_uint *si, dos_uint idx)
{ (void)rec; (void)idx; if (sprite_script_test_class_bit(si, 0x10) == 0) sprite_script_skip_bytes(si); return SS_CONTINUE; }

static enum sprite_script_control op_test_cursor_x_le(struct actor_record *rec, dos_uint *si, dos_uint idx)
{
    dos_uchar operand = sprite_script_fetch_byte_operand(si);
    dos_int val = (dos_int)(operand * 2);
    (void)rec; (void)idx;
    if ((dos_int)cursor_x <= val) sprite_script_skip_bytes(si);
    return SS_CONTINUE;
}
static enum sprite_script_control op_test_cursor_x_ge(struct actor_record *rec, dos_uint *si, dos_uint idx)
{
    dos_uchar operand = sprite_script_fetch_byte_operand(si);
    dos_uint val = (dos_uint)(operand * 2);
    (void)rec; (void)idx;
    if ((dos_uint)cursor_x >= val) sprite_script_skip_bytes(si);
    return SS_CONTINUE;
}
static enum sprite_script_control op_test_cursor_y_le(struct actor_record *rec, dos_uint *si, dos_uint idx)
{
    dos_uchar operand = sprite_script_fetch_byte_operand(si);
    (void)rec; (void)idx;
    if ((dos_int)cursor_y <= (dos_int)operand) sprite_script_skip_bytes(si);
    return SS_CONTINUE;
}
static enum sprite_script_control op_test_cursor_y_ge(struct actor_record *rec, dos_uint *si, dos_uint idx)
{
    dos_uchar operand = sprite_script_fetch_byte_operand(si);
    (void)rec; (void)idx;
    if ((dos_uint)cursor_y >= (dos_uint)operand) sprite_script_skip_bytes(si);
    return SS_CONTINUE;
}

static enum sprite_script_control op_random_skip(struct actor_record *rec, dos_uint *si, dos_uint idx)
{
    dos_uchar rand_lo = (dos_uchar)cc_rand();
    dos_uchar operand = sprite_script_fetch_byte_operand(si);
    (void)rec; (void)idx;
    if (rand_lo <= operand) sprite_script_skip_bytes(si);
    return SS_CONTINUE;
}

/* index == opcode byte == DATA_01037A_POINTER_TABLE[index]'s slot; see
 * the offset/label table in asm_sprites.h. */
static const sprite_script_handler_fn sprite_script_dispatch[SPRITE_SCRIPT_OPCODE_COUNT] = {
    op_set_pc,                     /*  0 : 0x4cb2 */
    op_add_offset,                  /*  1 : 0x4cbc */
    op_call,                         /*  2 : 0x4cc2 */
    op_return,                        /*  3 : 0x4ccb */
    op_timer_11,                       /*  4 : 0x4cd1 */
    op_timer_13,                        /*  5 : 0x4cd6 */
    op_timer_15,                         /*  6 : 0x4cdb */
    op_call_caf1,                         /*  7 : 0x4cef */
    op_dispatch_2a2d,                      /*  8 : 0x4cfa */
    op_dispatch_36f0,                       /*  9 : 0x4d0c */
    op_set_flag1,                            /* 10 : 0x4d17 */
    op_set_flag0,                             /* 11 : 0x4d1b */
    op_set_limits,                             /* 12 : 0x4d4e */
    op_set_velocity,                            /* 13 : 0x4d58 */
    op_move_clamped,                             /* 14 : 0x4d68 */
    op_set_position,                              /* 15 : 0x4dc5 */
    op_set_position_and_frame,                     /* 16 : 0x4de2 */
    op_flag8_set,                                   /* 17 : 0x4e0a */
    op_flag8_clear,                                  /* 18 : 0x4e15 */
    op_test_class_bits_7123_jz,                       /* 19 : 0x4e1c */
    op_test_class_bits_7123_jnz,                        /* 20 : 0x4e29 */
    op_test_class_bit_10_jnz,                            /* 21 : 0x4e36 */
    op_test_class_bit_10_jz,                              /* 22 : 0x4e43 */
    op_test_cursor_x_le,                                   /* 23 : 0x4e50 */
    op_test_cursor_x_ge,                                    /* 24 : 0x4e5e */
    op_test_cursor_y_le,                                     /* 25 : 0x4e6c */
    op_test_cursor_y_ge,                                      /* 26 : 0x4e78 */
    op_random_skip,                                            /* 27 : 0x4e92 */
};

void sprite_script_frame_driver(void)
{
    dos_uint count = actor_record_table[0];
    dos_uint idx;

    gfx_color_select(0x0F);

    for (idx = 0; idx < count; idx++) {
        struct actor_record *rec = (struct actor_record *)&actor_state_table[idx];
        dos_uint si;

        if ((dos_int)rec->board_id != board_record_index) continue; /* next_record */

        if (rec->rendered == 1) goto render_record;

        if (rec->countdown != 0) {
            rec->countdown--;
            continue; /* next_record */
        }

        si = rec->saved_pc;

    dispatch_opcode:
        {
            /* xor bx,bx; lodsb; mov bl,al; shl bx,1; add bx,74ah; jmp [bx]
             * -- i.e. `jmp word ptr [bx+074Ah]` through
             * DATA_01037A_POINTER_TABLE, unchecked. */
            dos_uchar op = SPRITE_SCRIPT_BASE[si]; si = (dos_uint)(si + 1); /* lodsb */
            enum sprite_script_control ctl;

            if (op < SPRITE_SCRIPT_OPCODE_COUNT) {
                ctl = sprite_script_dispatch[op](rec, &si, idx);
            } else {
                /* The historical jump has no bounds check: an opcode byte
                 * >= 28 would jump to whatever code happens to follow
                 * DATA_01037A_POINTER_TABLE's 28 words in DS -- not
                 * reproducible portably. Documented divergence: treated
                 * as a no-op continue rather than undefined behaviour. */
                ctl = SS_CONTINUE;
            }

            switch (ctl) {
            case SS_CONTINUE:
                goto dispatch_opcode;
            case SS_YIELD_RENDER:
                goto render_record;
            case SS_YIELD_NEXT:
            default:
                /* save_program_counter's board-id recheck (asm/SPRITES.ASM):
                 * resume at update_record only if the record (possibly just
                 * moved to a different board) still belongs to the current
                 * board. */
                if ((dos_int)rec->board_id == board_record_index) goto update_record;
                continue; /* next_record */
            }
        }

    render_record:
        if (rec->active == 1) continue; /* next_record */
        gfx_copy_rect(rec->x, rec->y, actor_record_bitmap(rec), rec->dir_flip);

    update_record:
        /* Also reached directly (goto) from the OP_MOVE_CLAMPED/
         * OP_SET_POSITION/OP_SET_POSITION_AND_FRAME/OP_FLAG8_SET opcodes'
         * shared save_program_counter tail, when the record's board id
         * (possibly just changed) still equals board_record_index -- see
         * asm/SPRITES.ASM's save_program_counter label. */
        if (rec->active == 1) continue; /* redundant vs. the check just above when reached from render_record -- present verbatim in the ASM */

        if (rec->vline_extra != 0) {
            /* mirrors board_actors_draw's identical strip copy, see the
             * note there about the apparently-swapped gfx_vline args. */
            gfx_vline((dos_int)(rec->x + 0x10), (dos_int)rec->vline_extra,
                      (dos_int)(rec->y - rec->vline_extra + 1));
        }

        /* collision_check: bounding box from actor_sprite_dims_table,
         * indexed by ((sprite_frame*2) with dir_flip byte-truncating-added,
         * then *4).  box_x1/x2/y1/y2 stay live (ax/cx/dx/bp in the ASM)
         * across check_special_flags into check_bounds below, exactly as
         * the ASM keeps them in registers -- hoisted to this scope rather
         * than recomputed. */
        {
            dos_int box_x1, box_x2, box_y1, box_y2;
            dos_uint dim_idx = (dos_uint)(rec->sprite_frame << 1);
            dos_uchar dim_lo = (dos_uchar)((dos_uchar)dim_idx + rec->dir_flip); /* 8-bit add, BL-only in the ASM -- truncation preserved */
            dos_uint dimA, dimB;

            dim_idx = (dos_uint)(((dim_idx & 0xFF00u) | dim_lo) << 2);

            dimA = (dos_uint)(actor_sprite_dims_table[dim_idx] | (actor_sprite_dims_table[(dos_uint)(dim_idx + 1)] << 8));
            dimB = (dos_uint)(actor_sprite_dims_table[(dos_uint)(dim_idx + 2)] | (actor_sprite_dims_table[(dos_uint)(dim_idx + 3)] << 8));

            box_x1 = (dos_int)((dos_uchar)dimA + rec->x);
            box_x2 = (dos_int)((dos_uchar)dimB + rec->x);
            box_y1 = (dos_int)((dos_uchar)(dimA >> 8) + rec->y);
            box_y2 = (dos_int)((dos_uchar)(dimB >> 8) + rec->y);

            /* Auxiliary two-corner bounds test against the cursor position,
             * table at DS:079E == b740[0x079E-0x0740], entry selected by
             * ((g72e<<1)+cursor_facing_left)<<2 (4 bytes/entry, 2 words).
             * `di` is only used here as a scratch copy of box_x1 in the
             * ASM (temporarily overwriting the record cursor) -- ported as
             * the already-named box_x1 local instead of literally
             * reusing a pointer. */
            {
                dos_uint entry = (dos_uint)(((dos_uint)(g72e << 1) + (dos_uint)cursor_facing_left) << 2);
                const dos_uchar *bounds = &b740[0x079E - 0x0740 + entry];
                dos_int b1_lo = (dos_int)(dos_uchar)bounds[0];
                dos_int b1_hi = (dos_int)(dos_uchar)bounds[1];
                dos_int b2_lo = (dos_int)(dos_uchar)bounds[2];
                dos_int b2_hi = (dos_int)(dos_uchar)bounds[3];
                int overlap = 1;

                if ((dos_int)(b1_lo + cursor_x) > box_x2) overlap = 0;
                else if ((dos_int)(b1_hi + cursor_y) > box_y2) overlap = 0;
                else if (box_x1 > (dos_int)(b2_lo + cursor_x)) overlap = 0;
                else if (box_y1 > (dos_int)(b2_hi + cursor_y)) overlap = 0;

                if (overlap) {
                    /* Latch the first colliding record this tick.  The ASM
                     * literally peeks the outer record_loop's saved CX
                     * (`pop bx; push bx`) without consuming it -- that
                     * value is (count - idx), the loop's REMAINING count,
                     * not a record index.  Reproduced literally; see the
                     * port report. */
                    if (g40ce == 0) {
                        g40ce = (dos_int)(dos_uint)(count - idx);
                    }
                }
            }

        /* check_special_flags: */
            if (rec->special_case != 0 && rec->special_handled != 1) {
                if (rec->special_case != 1 || (dos_uchar)hud_scroll_cooldown_ticks != 0) {
                    rec->call_return_pc = 0;
                    rec->timer_11 = 0;
                    rec->timer_13 = 0;
                    rec->timer_15 = 0;
                    rec->special_handled = 1;
                    rec->saved_pc = rec->restart_pc;
                }
            }

        /* check_bounds: level-bounds clamp gated by raycast_trail_active
         * and only when not already delayed; re-arms the countdown from
         * reload_delay when the (tx[gc04e],ty[gc04e]) reference point
         * lies inside this record's own collision box. */
            if (raycast_trail_active != 0 && rec->countdown == 0) {
                dos_int tx_val = tx[gc04e];
                if (tx_val >= box_x1 && tx_val <= box_x2) {
                    dos_int ty_val = ty[gc04e];
                    if (ty_val >= box_y1 && ty_val <= box_y2) {
                        rec->countdown = rec->reload_delay;
                    }
                }
            }
        }

        continue; /* advance_record -> next_record */
    }
}
