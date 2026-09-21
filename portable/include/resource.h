/* resource.h -- portable AE000/AE001 archive access (src/RESOURCE.C).
 *
 * Packed index: top 4 bits = directory (0 = AE000, 1 = AE001, 2 = AE002),
 * low 12 bits = record number.  The archive file is a little-endian table of
 * 32-bit absolute offsets (first offset == 4 * table entries, and equals the
 * historical "stamp" ga52[dir]) followed by the records; each record is
 * [type byte][flags byte][payload].
 *
 * Staging buffers keep the historical aliasing: one block of 0xFA80 bytes,
 * ui_gfx_blob = block+0x0E, ui_gfx_shadow_a = ui_gfx_blob+2, ui_gfx_shadow_b =
 * ui_gfx_shadow_a + 0x7D30 (src/PLAYERSL.C ui_gfx_alloc).  The record is read
 * at ui_gfx_blob, so the header lands at ui_gfx_blob[0..1] and the payload at
 * ui_gfx_shadow_a; decoded output ends in ui_gfx_shadow_a.
 */
#ifndef PORTABLE_RESOURCE_H
#define PORTABLE_RESOURCE_H

#include "dos_types.h"

#define UI_GFX_BLOCK_BYTES     0xFA80u
#define UI_GFX_BLOB_OFFSET     0x0Eu
#define UI_GFX_SHADOW_A_OFFSET 0x10u
#define UI_GFX_SHADOW_B_OFFSET 0x7D40u

extern uint8_t *ui_gfx_blob;      /* DS:C5CA */
extern uint8_t *ui_gfx_shadow_a;  /* DS:C5C6 */
extern uint8_t *ui_gfx_shadow_b;  /* DS:C5BE */

/* Historical globals the loader writes (declared here until game_state.h
 * takes them over; the definitions live in portable/resource/archive.c). */
extern dos_char gc0cb;            /* DS:C0CB -- last record's type byte */
extern dos_char display_mode;     /* DS:BFCD -- 1..5; the port runs 4 */

/* Point the loader at the directory holding AE000.DAT/AE001.DAT.  Files are
 * looked up by the historical names (drive prefix dropped); a case-insensitive
 * fallback is applied at open time. */
void resource_set_asset_dir(const char *dir);

/* Allocate the staging block (src/PLAYERSL.C ui_gfx_alloc). */
void resource_staging_init(void);
void resource_staging_shutdown(void);

/* Open directory `dir` (0..2).  Returns 0 on success, -1 if the file cannot
 * be opened or its first dword does not equal the historical stamp. */
int resource_archive_open(int dir);

/* src/RESOURCE.C resource_load_record: read record `packed`, apply the flag
 * dispatch (flags&2 -> lz, flags&1 -> rle, both -> lz then rle) and the type
 * dispatch (0x47 sprite, 0 sequential sheet, 1 indexed sheet) unless
 * display_mode == 5.  Returns the historical 16-bit `s` (decoded length as
 * computed by the original, including its `(int)o2 - (int)o1` truncation). */
dos_int resource_load_record(dos_uint packed);

/* Wrappers.  _alloc mallocs n bytes and copies; on allocation failure it
 * reports through the failure handler and exits.  _into copies n bytes to q. */
void resource_load_record_alloc(dos_int packed, uint8_t **pp);
void resource_load_record_into(dos_uint packed, uint8_t *q);

/* Raw access used by tests and tools: returns a pointer into the archive's
 * in-memory image for record `index` of directory `dir` and its length
 * (the full 32-bit end-start), or NULL if out of range. */
const uint8_t *resource_record_raw(int dir, unsigned index, uint32_t *len);
unsigned resource_record_count(int dir);

/* Failure policy.  The historical loader blocked in a modal dialog and
 * retried; the port reports through this hook (default: message on stderr
 * and exit(1)).  Read failures are retried by the caller policy; save-slot
 * writes fail outright (docs/current/portability-boundaries.md section 9). */
typedef void (*resource_failure_fn)(int dir, const char *what);
void resource_set_failure_handler(resource_failure_fn fn);

#endif
