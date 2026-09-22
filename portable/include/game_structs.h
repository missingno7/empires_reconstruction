/* game_structs.h -- hand-ported historical record types.
 *
 * Source of truth: include/C470.H, DIALOG.H, G0DCC.H, G2FD2.H, GA5E.H,
 * GB3AF.H, GC0FE.H, GC316.H, GC91B.H, R3E8.H, TBL.H (historical tree, not
 * edited). Field names, byte offsets and comments are carried over from
 * those headers; only the field *types* change (dos_* aliases, plain
 * pointers instead of near/far).
 *
 * `#pragma pack(push,1)` covers the pointer-free records whose historical
 * byte layout is directly visible to the game (i.e. code reaches into them
 * as a raw byte buffer, or their extent is asserted against a fixed byte
 * count elsewhere): c470_record (27 bytes), g0dcc_entry (24 bytes),
 * g2fd2_entry (2 bytes), ga5e_entry (0x23 bytes), gb3af_entry (32 bytes),
 * gc316_tile (2 bytes), gc91b_entry (14 bytes), record3e8 (1000 bytes),
 * tbl_entry (27 bytes). `struct dialog` and `struct gc0fe_record`/
 * `gc0fe_catalog` hold real pointers (`char far *` -> a native pointer,
 * historically 4 bytes, now 8) and are therefore NOT packed to a
 * historical byte count -- their historical DATA/BSS instances are placed
 * by DS-offset span, not by `sizeof`, so the wider pointer does not shift
 * any other object.
 *
 * portable/generated/game_data.[ch] and game_state.[ch] include this
 * header whenever a generated object's historical type resolves to one of
 * these structs.
 */
#ifndef PORTABLE_GAME_STRUCTS_H
#define PORTABLE_GAME_STRUCTS_H

#include "dos_types.h"

/* ---- include/C470.H --------------------------------------------------
 * 27-byte save-slot record. DS:C470 holds the ten live slots (slot_table),
 * DS:C360 the ten transfer copies (slot_transfer_table); both are BSS
 * tables. Offsets and widths are established by the code in the files
 * named below; names describe observed use, not recovered text.
 */
#pragma pack(push, 1)
struct c470_record {
    dos_char text[9];           /* +00  name; text[0]==0 marks a free slot (F_A223, F_AD25) */
    dos_int value;               /* +09  score-like counter: ++ in F_3986, ultoa in F_A28D, =1 in F_ADCF */
    dos_char flags;              /* +0B  bit 0x10 selects the Explorer/Expert label (F_A28D);
                                          bit 0x20 tested by F_ACE7 and set by F_ADCF; bits 0..4 or-ed by F_CE68 */
    dos_char resume_round;       /* +0C  1-based round index to resume, 0 = none: written as i+1 by
                                          campaign_chapter_advance (F_4943.C) for the round being played, cleared by
                                          F_D26C and slot_reset_for_new_game; read (then decremented back to 0-based)
                                          by F_49F0/GAME.C to auto-resume that round at game start */
    dos_int sound;                /* +0D  copied to sound_enabled by F_AB66 and M_CB5C_CD23 */
    dos_int music;                /* +0F  copied to music_enabled by F_AB66 and M_CB5C_CD23 */
    dos_int option;               /* +11  0/1 toggle in M_CB5C_CD23; nonzero gate in F_D4B3 */
    dos_int pending;              /* +13  bit mask: set to -1 by M_CB5C_CD23, bits cleared by F_D4B3 */
    dos_char state;              /* +15  4 written by F_3986, F_9DCC and F_ADCF; passed to energy_set by F_49F0 */
    dos_char round_progress[4];  /* +16  per-round progress counters: campaign_chapter_advance (F_4943.C) computes
                                          campaign_round_node_cursor = round_progress[i]*2+i*8 for round i and increments
                                          round_progress[i] as level_driver_run() reports success */
    dos_char byte26;             /* +1A  cleared by F_ADCF; otherwise unused */
};
#pragma pack(pop)

/* ---- include/DIALOG.H --------------------------------------------------
 * Dialog descriptor shared by dialog_layout (geometry), dialog_draw
 * (drawing) and dialog_run (input loop). 20 bytes historically (byte
 * packed, far pointers); the ten-or-so static descriptors in DATA all
 * carry cx=cy=w=lines=-1. Names describe observed use, not recovered text.
 * NOT byte-packed here: title/text are now native pointers (8 bytes on
 * x64, was a 4-byte far pointer), so sizeof(struct dialog) differs from
 * the historical 20; every DATA/BSS instance is placed by DS-offset span,
 * not by sizeof(struct dialog), so this is safe.
 */
struct dialog {
    dos_int kind;                 /* +00  box kind 0..7; selects geometry and buttons */
    dos_char *title;              /* +02  optional heading string; 0 = none */
    dos_char sub;                 /* +06  button-set selector 0..2 (dialog_layout) */
    dos_char *text;               /* +07  body text; CR or LF separates lines */
    dos_uchar initial;            /* +0B  initial selection/direction for kinds 4..7 */
    dos_int cx;                   /* +0C  requested x; -1 = centre horizontally */
    dos_int cy;                   /* +0E  requested y; -1 = centre vertically */
    dos_int w;                    /* +10  requested width; -1 = measure the text */
    dos_int lines;                /* +12  requested line count; -1 = count the text */
};

/* ---- include/G0DCC.H ----------------------------------------------------
 * g0dcc[40]: 24-byte records at DS:0DCC ("40 records of 24 bytes at
 * DS:0DCC", layout/production-plan.json). F_9A0E walks each record as 11
 * (index, selector) byte pairs; F_9B68 tests byte 22 as a bitmask; F_9DCC
 * reads byte 23 as a signed frame offset.
 */
#pragma pack(push, 1)
struct g0dcc_entry {
    struct {
        dos_uchar idx;  /* F_9A0E: far-pointer-table index (was e[j].a) */
        dos_char sel;   /* F_9A0E: signed handler selector (was e[j].b) */
    } e[11];            /* offsets 0..21 */
    dos_char f16;       /* offset 22: F_9B68 bit-tested byte (signed, matches original codegen) */
    dos_char f17;       /* offset 23: F_9DCC signed frame offset (P2.b at index 11) */
};
#pragma pack(pop)

/* ---- include/G2FD2.H -----------------------------------------------------
 * ui_panel_glyph_records[]: 2-byte records at DS:2FD2. F_DA66 reads both
 * bytes as glyph indices; F_DD72 reads both as tone-table indices
 * (voice_set_field arguments); M_DAD7_DB35 reads byte 1 as a
 * state/selector index (voice_level_table/voice_byte_table lookup).
 */
#pragma pack(push, 1)
struct g2fd2_entry {
    dos_char b0;   /* offset 0: F_DA66 .a / F_DD72 .a */
    dos_char b1;   /* offset 1: F_DA66 .b / F_DD72 .b / M_DAD7_DB35 .selector */
};
#pragma pack(pop)

/* ---- include/GA5E.H -------------------------------------------------------
 * ga5e[]: 35-byte (0x23) directory records at DS:0A5E (DGROUP offset 2654).
 * F_6266 and F_656C only ever take the address of a whole record (never
 * dereference a field); F_643A reaches into the same blob as a flat byte
 * buffer (message at DGROUP:0AC7 is ga5e+0x69), so it keeps its arithmetic
 * in bytes via a cast rather than indexing by record.
 */
#pragma pack(push, 1)
struct ga5e_entry { dos_char b[0x23]; };
#pragma pack(pop)

/* ---- include/GB3AF.H -------------------------------------------------------
 * actor_state_table[]: 32-byte board-cell records at DS:B3AF (DGROUP offset
 * 45999). F_B99F names the shape struct Record {flag; rest[31]} and also
 * reaches into it as a flat near byte buffer for cell indices that are not
 * record-aligned; F_338A and F_B7F9 only ever touch it as that same flat
 * byte buffer, so they keep byte indexing via a cast to the flat form.
 */
#pragma pack(push, 1)
struct gb3af_entry { dos_uchar flag; dos_char rest[31]; };
#pragma pack(pop)

/* ---- include/GC0FE.H -------------------------------------------------------
 * gc0fe: a far pointer at DS:0FE (DGROUP offset 49406) that different
 * subsystems repurpose. F_7932 only ever stores or copies the pointer
 * value (as char far *); F_7964/F_7D91 dereference it as the 20-byte
 * record catalog below, cast at the point of use.
 *
 * F_7BFC also dereferences gc0fe, but as an unrelated {int n; struct
 * catalog_entry *p} shape that does not agree byte-for-byte with the
 * catalog record below -- a genuine conflict, not unified here; that
 * local shape stays out of this header (not part of the portable ABI).
 */
/* Unified with include/GC0FE.H's F_7964 view AND src/MENULIST.C's
 * `struct catalog_entry {char *a; int b; char pad[12]; int c;}` view of the
 * SAME 20-byte historical record (offsets 0..5 -- three opaque ints there,
 * `h0,h1,h2` -- are really MENULIST.C's `.a` (a far pointer, 4 bytes) and
 * `.b` (2 bytes); every other field already agreed byte-for-byte, hence
 * the shared name/offsets below). `gc0fe_record`/`gc0fe_catalog` are kept
 * as macro aliases for any historical declaration still spelled that way.
 */
struct menu_record {
    dos_char *label;                /* offset 0: MENULIST .a (F_7964 .h0/.h1 as one far ptr) */
    dos_int label_width;            /* offset 4: MENULIST .b (F_7964 .h2) */
    dos_int count;                  /* offset 6: row count (F_7964 .count / F_7D91 .d, dialog lines) */
    dos_char *text;                 /* offset 8: label text (F_7964 .middle / F_7D91 .p, dialog text) */
    void (**callbacks)(void);       /* offset 12: far pointer to an array of handler code pointers
                                        (F_7964 .callbacks; unused by F_7D91) */
    dos_int width;                  /* offset 16: box width (F_7964 .width / F_7D91 .g, dialog w) */
    dos_int x;                      /* offset 18: box x (F_7964 .x / F_7D91 .h, dialog cx); MENULIST .c */
};
#define gc0fe_record menu_record

/* The catalog gc0fe points at when used this way: a record count and a far
 * pointer to the record array (F_7964's struct catalog). */
struct menu_catalog {
    dos_int count;                 /* offset 0 */
    struct menu_record *records;   /* offset 2 */
};
#define gc0fe_catalog menu_catalog

/* ---- include/GC316.H -------------------------------------------------------
 * puzzle_grid[4][6]: the 4x6 slot grid, one 2-byte tile per cell (DS:C316).
 * F_8A37 names the fields a/b, F_8AA2 (via ROW.e[6]) a/b (unsigned),
 * F_8BAB (struct Piece) kind/rotation, F_90A6 (struct piece_desc) a/b.
 */
#pragma pack(push, 1)
struct gc316_tile {
    dos_char kind;  /* offset 0: F_8A37 .a / F_8AA2 .a / F_8BAB .kind (signed; F_8AA2 casts to unsigned
                        for its 0xff sentinel compare) / F_90A6 .a */
    dos_char rot;   /* offset 1: F_8A37 .b / F_8AA2 .b / F_8BAB .rotation / F_90A6 .b */
};
#pragma pack(pop)

/* ---- include/GC91B.H -------------------------------------------------------
 * voice_param_record[]: 14-byte per-voice OPL records at DS:C91B. F_E095
 * indexes the whole record by a runtime member index; F_E0C0/F_E27B/
 * F_E2D6/F_E324/F_E44B name the members c0..c13, F_E1F2/F_E372 name them
 * f0..fd -- all char, same 14 offsets, so this header keeps one flat
 * indexable array and every file addresses it by its own constant/variable
 * index.
 */
#pragma pack(push, 1)
struct gc91b_entry {
    dos_char f[14];  /* offsets 0..13, per-voice OPL record bytes */
};
#pragma pack(pop)

/* ---- include/R3E8.H -------------------------------------------------------
 * Recovered 1000-byte (0x3E8) record; DS:43B4 contains ten consecutive
 * entries.
 */
#pragma pack(push, 1)
struct record3e8 {
    dos_uchar bytes[0x3e8];
};
#pragma pack(pop)

/* ---- include/TBL.H -------------------------------------------------------
 * tbl[]: 27-byte records at DS:C4F0 (DGROUP offset 50288). F_A85E names
 * every offset it touches; F_68CF only touches offsets 13 and 15, which
 * F_A85E's d13/f15 already name and which agree in type (both int).
 */
#pragma pack(push, 1)
struct tbl_entry {
    dos_char pad0[9];
    dos_int  a9;        /* +9  */
    dos_char b11;       /* +11 */
    dos_char pad1;
    dos_int  d13;        /* +13: F_A85E .d13 / F_68CF .a */
    dos_int  f15;        /* +15: F_A85E .f15 / F_68CF .b */
    dos_int  h17;        /* +17 */
    dos_int  j19;        /* +19 */
    dos_char l21;        /* +21 */
    dos_char pad2[5];
};                       /* sizeof == 27 historically */
#pragma pack(pop)

#endif /* PORTABLE_GAME_STRUCTS_H */
