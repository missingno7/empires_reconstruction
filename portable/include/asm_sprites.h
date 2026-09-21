/* asm_sprites.h -- shared record layouts for asm/SPRITES.ASM
 * (play_window_wipe_clipped, sprite_script_frame_driver,
 * sprite_table_wipe_active, board_actors_draw -- portable/game/asm_sprites.c)
 * and asm/SPRDRAW.ASM (sprite_table_queue_draws, animated_tile_tick,
 * sprite_record_adjust_draw -- portable/game/asm_sprdraw.c).
 *
 * ---- storage identity findings (state-map.md), not obvious from either
 * ASM file's own `extrn` list -----------------------------------------
 *
 * 1. SPRDRAW.ASM's `_objtab` and `_g40d0` externs are historically THE SAME
 *    OBJECT: state-map.md resolves both to one generated symbol, `objtab`
 *    (DS:40D0, `dos_char *`, alias `g40d0` -- portable/generated/game_state.h
 *    line 17-19).  sprite_table_queue_draws()/sprite_record_adjust_draw()
 *    (which the asm-module-inventory.md Wave-2 spec calls "objtab, a sprite
 *    object table") and animated_tile_tick() (which the same doc calls
 *    "g40d0, the animated-tile table") therefore walk the identical far
 *    pointer's target array; the doc's own text flags the two as "review"
 *    because the scan that wrote it never found the shared extern -- this
 *    header records the resolution.  Both are ported against the one
 *    generated `objtab` (portable/game/asm_sprdraw.c).
 * 2. Likewise `_sprbase` (SPRDRAW.ASM's F_6036/F_6181) and `_sprite_tile_bank`
 *    (F_60A9/animated_tile_tick) are the same object: `sprite_tile_bank`
 *    (DS:BFC8, `dos_char *`, alias `sprbase` -- game_state.h line 243-246).
 * 3. SPRITES.ASM's raw displacement `0BFBAh` used inside
 *    sprite_script_frame_driver IS `board_record_index` (DS:BFBA,
 *    game_state.h line 226-228) -- the same object SPRITES.ASM's other two
 *    routines reach through the `_board_record_index` extern.  Also
 *    resolves a "review" flag from the inventory doc.
 * 4. SPRITES.ASM's raw displacement `0B3AEh` (the bytecode-interpreter's
 *    "own" 0x20-byte-stride table, per the inventory doc's description) IS
 *    `_actor_record_table` (DS:B3AE), the SAME table
 *    sprite_table_wipe_active()/board_actors_draw() walk through the
 *    `_actor_record_table` extern -- there is only one such table, not two.
 *    `actor_record_table[0]` (game_state.h line 207-210) is the record
 *    count; the 0x20-byte records themselves are `actor_state_table[12]`
 *    (DS:B3AF, `struct gb3af_entry[12]`, game_state.h line 212-214),
 *    immediately following.  This resolves the inventory doc's "review"
 *    flag on this address; see `struct actor_record` below for the field
 *    breakdown recovered from the ASM (game_structs.h's own
 *    `struct gb3af_entry` is intentionally opaque -- one flag byte plus 31
 *    raw bytes -- because no *other* ported module needs more detail yet).
 * 5. `raycast_trail_active` (DS:08FE) gates BOTH animated_tile_tick()'s
 *    entire body (SPRDRAW.ASM) AND sprite_script_frame_driver()'s
 *    check_bounds level-clamp step (SPRITES.ASM) -- the same flag, ported
 *    from two different files.
 */
#ifndef PORTABLE_ASM_SPRITES_H
#define PORTABLE_ASM_SPRITES_H

#include "dos_types.h"

/* ---- asm/SPRITES.ASM: the 0x20-byte actor/script record ---------------
 *
 * Recovered field-by-field from every `[di+N]` access across all three
 * SPRITES.ASM routines that touch actor_record_table/actor_state_table
 * (play_window_wipe_clipped does not; it only receives already-extracted
 * words).  `actor_record_table[0]` is the record count; record i lives at
 * `actor_state_table[i]`, reinterpreted through this overlay exactly as
 * game_structs.h's own header comment describes for the other packed
 * historical structs in this codebase ("code reaches into them as a raw
 * byte buffer").  sizeof(struct actor_record) == sizeof(struct
 * gb3af_entry) == 32 (checked below with a compile-time assertion).
 *
 * Offsets 0x1C..0x1F (the last 4 bytes of the record) are never read or
 * written by any instruction in asm/SPRITES.ASM or asm/SPRDRAW.ASM; kept
 * as `reserved` rather than omitted so the struct's size still matches.
 */
#pragma pack(push, 1)
struct actor_record {
    dos_uchar rendered;       /* +0x00 F_4B0C record_loop: ==1 -> render_record fast path, skip the bytecode program this tick */
    dos_uchar board_id;       /* +0x01 owning board id; all three routines skip records where this != board_record_index */
    dos_int   x;               /* +0x02 */
    dos_int   y;               /* +0x04 */
    dos_uchar sprite_frame;    /* +0x06 current frame/sprite index -- indexes tile_height_table/tile_width_table (F_4E9F), resource_ptr_table (F_4EEB/F_4B0C), and (with dir_flip) actor_sprite_dims_table (F_4B0C collision box) */
    dos_uchar dir_flip;        /* +0x07 secondary index / gfx_copy_rect flip argument; also the collision dims-table's low-byte-truncating-add operand */
    dos_uchar active;          /* +0x08 master "skip this record's draw and collision" flag -- checked identically (==1 -> skip) by board_actors_draw, sprite_table_wipe_active and F_4B0C's own render_record/update_record */
    dos_uchar reload_delay;    /* +0x09 value check_bounds copies into countdown (+0x0A) when the level-bounds clamp re-arms this record */
    dos_uchar countdown;       /* +0x0A per-tick delay counter; nonzero -> record_loop just decrements it and moves on without resuming the bytecode program */
    dos_uchar frame_limit_lo;  /* +0x0B opcode_set_limits low bound (also opcode_move_clamped's wrap-to-hi threshold) */
    dos_uchar frame_limit_hi;  /* +0x0C opcode_set_limits high bound (also opcode_move_clamped's wrap-to-lo threshold) */
    dos_uint  saved_pc;        /* +0x0D..0x0E resumable bytecode offset, relative to actor_record_table's own address -- see SPRITE_SCRIPT_BASE below */
    dos_uint  call_return_pc;  /* +0x0F..0x10 single-level subroutine return offset (opcode_store_offset/opcode_restore_offset) */
    dos_uint  timer_11;        /* +0x11..0x12 countdown-timer opcode slot #1 (bx=0x11 variant) */
    dos_uint  timer_13;        /* +0x13..0x14 countdown-timer opcode slot #2 (bx=0x13 variant) */
    dos_uint  timer_15;        /* +0x15..0x16 countdown-timer opcode slot #3 (bx=0x15 variant) */
    dos_uint  restart_pc;      /* +0x17..0x18 bytecode offset reset_fields resumes at after a collision special-case */
    dos_uchar special_case;    /* +0x19 check_special_flags selector (0 = none; 1 is gated additionally by hud_scroll_cooldown_ticks' low byte; any other nonzero value is unconditional) */
    dos_uchar vline_extra;     /* +0x1A optional extra gfx_vline strip length -- read identically by F_4B0C's update_record and F_4EEB */
    dos_uchar special_handled; /* +0x1B "already ran the collision special case" gate; cleared by the opcode that sets `active` (byte+0x08) to 1 */
    dos_uchar reserved[4];     /* +0x1C..0x1F never referenced */
};
#pragma pack(pop)

_Static_assert(sizeof(struct actor_record) == 32, "actor_record must match struct gb3af_entry's 32 bytes");

/* SPRITE_SCRIPT_BASE -- the fixed reference address (DS:B3AE ==
 * &actor_record_table[0]) the bytecode interpreter's saved/relative program
 * counters (struct actor_record::saved_pc/call_return_pc/restart_pc, and
 * the interpreter's own local "si" cursor) are offsets from.
 *
 * IMPORTANT, flagged for the supervisor: this is NOT the base of a real
 * bytecode buffer this port owns.  Historically `si = 0xB3AE + offset` can
 * address literally anywhere in the 64 KiB DS segment (the 16-bit add
 * wraps); the actual sprite-script bytecode programs are evidently NOT
 * stored inside actor_state_table itself (each record's own unidentified
 * tail is only 4 bytes, offsets 0x1C..0x1F -- nowhere near enough for a
 * real program), so they must live in some OTHER, not-yet-identified
 * runtime/resource-owned buffer that the historical build happened to
 * place within 64 KiB of DS:B3AE.  No generated symbol or docs/current
 * artifact found by this port names that buffer.  This header preserves
 * the historical addressing convention (offsets are added to
 * actor_record_table's own address, exactly like the ASM's `add
 * si,0b3aeh`) so that whichever module eventually owns real bytecode
 * storage can place it correctly relative to this same base; until then,
 * `sprite_script_frame_driver()` can only be exercised with synthetic
 * programs the caller places itself (see
 * portable/tests/test_asm_sprites.c) at a small, deliberately-chosen
 * offset from actor_record_table (e.g. inside a spare actor_state_table
 * slot the test does not otherwise use).
 */
#define SPRITE_SCRIPT_BASE ((dos_uchar *)actor_record_table)

/* Bytecode dispatch opcode set (asm/SPRITES.ASM sprite_script_frame_driver,
 * F_4B0C).  VERIFIED against the real table:
 *
 * The dispatcher is (F_4B0C, 0x4B4D-0x4B51): `add bx,74Ah` / `jmp [bx]`
 * with bx already holding opcode*2 (`xor bx,bx; lodsb; mov bl,al; shl
 * bx,1`) -- i.e. `jmp word ptr [bx+074Ah]`, an unchecked zero-extended-
 * byte-times-2 index with NO range test: any opcode byte >= 28 walks off
 * the end of the table and jumps to whatever word happens to sit past it
 * (transcribed as-is below; sprite_script_dispatch[] only defines entries
 * 0..27 and a bytecode program is trusted never to emit anything higher,
 * exactly as the historical code trusted it).
 *
 * The table itself IS in the tree: `src/data/DATA_01037A_POINTER_TABLE.json`
 * (DS:074A, 28 x u16 code offsets, `"format": "u16le-table-v1"`) --
 * confirmed by disassembling asm/SPRITES.ASM's F_4B0C member
 * (build/F_4B0C.ndisasm.txt, module M_4AA8_4EEB starting at 0x4AA8) and
 * matching each of the 28 table offsets against a real label address:
 *
 *   idx  offset  label (build/F_4B0C.ndisasm.txt address)
 *   0    0x4cb2  set_program_counter
 *   1    0x4cbc  opcode_add_offset
 *   2    0x4cc2  opcode_store_offset (CALL)
 *   3    0x4ccb  opcode_restore_offset (RETURN)
 *   4    0x4cd1  opcode_countdown, timer slot +0x11
 *   5    0x4cd6  opcode_countdown, timer slot +0x13
 *   6    0x4cdb  opcode_countdown, timer slot +0x15
 *   7    0x4cef  opcode_call_caf1 (call 0xcaf1 = _stream_control_block_arm)
 *   8    0x4cfa  opcode_dispatch_2a2d (call 0x2a2d, then call 0x338a)
 *   9    0x4d0c  opcode_dispatch_36f0 (call 0x36f0)
 *   10   0x4d17  opcode_set_flag, dl=1
 *   11   0x4d1b  opcode_set_flag, dl=0
 *   12   0x4d4e  opcode_set_limits
 *   13   0x4d58  opcode_set_velocity
 *   14   0x4d68  opcode_move_clamped
 *   15   0x4dc5  opcode_set_position
 *   16   0x4de2  opcode_set_position_and_frame
 *   17   0x4e0a  opcode_enable (sets byte+8=1, byte+0x1b=0)
 *   18   0x4e15  opcode_disable (sets byte+8=0)
 *   19   0x4e1c  test class-flag bits 0x07, jz -> skip
 *   20   0x4e29  test class-flag bits 0x07, jnz -> skip
 *   21   0x4e36  test class-flag bit 0x10, jnz -> skip
 *   22   0x4e43  test class-flag bit 0x10, jz -> skip
 *   23   0x4e50  cursor_x <= operand*2 (signed) -> skip
 *   24   0x4e5e  cursor_x >= operand*2 (unsigned) -> skip
 *   25   0x4e6c  cursor_y <= operand (signed) -> skip
 *   26   0x4e78  cursor_y >= operand (unsigned) -> skip
 *   27   0x4e92  opcode_random_skip (call 0xf90f = _rand)
 *
 * `opcode_skip_bytes` (0x4e84, "si += b740[0x782-0x740+operand]") is NOT
 * itself a table entry -- every "-> skip" row above (19-26) and
 * opcode_random_skip (27) jump/fall INTO 0x4e84 as a shared internal
 * subroutine, never dispatched to directly.  This corrects the previous
 * revision of this header, which mistook it for a 29th opcode (dispatch
 * table has exactly 28 entries, matching DATA_01037A_POINTER_TABLE.json's
 * 28 values); ported as the static helper sprite_script_skip_bytes() in
 * asm_sprites.c rather than as an enum value.
 */
enum sprite_script_opcode {
    OP_SET_PC = 0,
    OP_ADD_OFFSET = 1,
    OP_CALL = 2,
    OP_RETURN = 3,
    OP_TIMER_11 = 4,
    OP_TIMER_13 = 5,
    OP_TIMER_15 = 6,
    OP_CALL_CAF1 = 7,
    OP_DISPATCH_2A2D = 8,
    OP_DISPATCH_36F0 = 9,
    OP_SET_FLAG1 = 10,
    OP_SET_FLAG0 = 11,
    OP_SET_LIMITS = 12,
    OP_SET_VELOCITY = 13,
    OP_MOVE_CLAMPED = 14,
    OP_SET_POSITION = 15,
    OP_SET_POSITION_AND_FRAME = 16,
    OP_FLAG8_SET = 17,          /* historically labelled "opcode_enable" but see asm_sprites.c's note -- byte+0x08==1 is what ALL THREE draw routines treat as "skip this record" */
    OP_FLAG8_CLEAR = 18,        /* historically labelled "opcode_disable" -- see the same note */
    OP_TEST_CLASS_BITS_7123_JZ = 19,
    OP_TEST_CLASS_BITS_7123_JNZ = 20,
    OP_TEST_CLASS_BIT_10_JNZ = 21,
    OP_TEST_CLASS_BIT_10_JZ = 22,
    OP_TEST_CURSOR_X_LE = 23,
    OP_TEST_CURSOR_X_GE = 24,
    OP_TEST_CURSOR_Y_LE = 25,
    OP_TEST_CURSOR_Y_GE = 26,
    OP_RANDOM_SKIP = 27,
    SPRITE_SCRIPT_OPCODE_COUNT = 28
};

/* ---- asm/SPRDRAW.ASM: the 3-byte packed sprite/animated-tile record ---
 *
 * `objtab`/`g40d0` (see the storage-identity note above) and `sprbase`/
 * `sprite_tile_bank` share ONE 3-byte-record, length-prefixed table
 * format: count byte, then 3-byte records of (x byte, y byte, flags byte).
 * Fields kept as three plain bytes rather than a packed x/y word to avoid
 * any host-endianness assumption -- the ASM itself decodes the record's
 * first word by splitting it into two bytes (`mov dl,ah` / `mov bl,al`),
 * never by reading it as a native 16-bit value, so this struct matches
 * that decoding literally.
 *
 * flags: bit 7 = armed (only tested by animated_tile_tick's g40d0 walk;
 * sprite_table_queue_draws/sprite_record_adjust_draw's objtab walk never
 * tests it), bit 6 = frame direction (0 = count up, 1 = count down),
 * bits 4..0 = current frame, 0..0x17 (23) -- both animated_tile_tick and
 * sprite_record_adjust_draw advance this field by 1 with wraparound at
 * the two ends; sprite_table_queue_draws only reads it, never advances it.
 */
#pragma pack(push, 1)
struct sprite_anim_record {
    dos_uchar x;
    dos_uchar y;
    dos_uchar flags;
};
#pragma pack(pop)

_Static_assert(sizeof(struct sprite_anim_record) == 3, "sprite_anim_record must match the ASM's 3-byte record stride");

#define SPRITE_ANIM_FLAG_ARMED     0x80u
#define SPRITE_ANIM_FLAG_DIR_DOWN  0x40u
#define SPRITE_ANIM_FRAME_MASK     0x1Fu
#define SPRITE_ANIM_FRAME_MAX      0x17u /* frame wraps at 0 and 0x17 (23), not 0x1F */

/* Frame offset into a sprite/tile bank far pointer (sprbase/
 * sprite_tile_bank), per asm-module-inventory.md and confirmed by every
 * `shl bx,1 x2 ... les/lds` site in both ASM files: frame*0x1E6 + 2. */
#define SPRITE_FRAME_STRIDE 0x1E6
#define SPRITE_FRAME_HEADER 2

#endif /* PORTABLE_ASM_SPRITES_H */
