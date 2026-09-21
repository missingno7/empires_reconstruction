/* recttab.c -- src/RECTTAB.C: the rectangle-hit table (grouped module
 * C_D85F_D8F0; the three members below kept their original section ids).
 *
 * g2f30 is a byte table: g2f30[0] = record count, followed by up to that
 * many 5-byte records [id][x][y][w][h] (src/RECTTAB.C's own asm comments;
 * confirmed by F_D85F's record stride and F_D89A's two lodsw reads per
 * record below).
 */
#include "game.h"

/* ---- F_D85F (original code at 0xD85F) ---- */
/* F_D85F -- delete the matching five-byte record and compact the table.
 *
 * PORT: transcribed from the inline asm body (loop over g2f30's records
 * comparing each record's id byte against `key`'s low byte; on a match,
 * shift every following record down by one slot with an overlapping
 * forward copy -- safe here because the destination trails the source by
 * exactly one record -- and decrement the count byte). */
void record_table_delete_compact(dos_int key)
{
    dos_uchar count = g2f30[0];
    dos_uchar k;

    for (k = 0; k < count; k++) {
        dos_uchar *rec = g2f30 + 1 + (dos_uint)k * 5;
        if (rec[0] == (dos_uchar)key) {
            dos_uchar after = (dos_uchar)(count - k - 1);
            if (after)
                memmove(rec, rec + 5, (size_t)after * 5);
            g2f30[0] = (dos_uchar)(count - 1);
            return;
        }
    }
}

/* ---- F_D89A (original code at 0xD89A) ---- */
/* F_D89A -- return the first rectangle-table id overlapping the query rect
 * (x,y,w,h), or 0 if none overlaps.  Asm body under a TC frame (see the
 * historical file's own comment: lodsb/lodsw scanning with loop and BP as
 * a data register are not Turbo C 2.0 C-codegen shapes).
 *
 * PORT: x/y/w/h are truncated to their low byte exactly as the asm's
 * byte-register loads do (mov bl,al etc. from each word-sized stack arg).
 * The far-corner sums (x+w, y+h for the query; rx+rw, ry+rh for each
 * record) are computed with the SAME packed 16-bit add the asm uses
 * (ADD DX,BX / ADD AX,BP): the two bytes are packed into one dos_uint and
 * added as one 16-bit op so the carry out of the low byte propagates into
 * the high byte exactly as it does on the 8086, then each byte is read
 * back separately.  Byte-significant: do not split this into two
 * independent byte additions. */
dos_int rect_table_hit_id(dos_int x, dos_int y, dos_int w, dos_int h)
{
    dos_uchar x1 = (dos_uchar)x;
    dos_uchar y1 = (dos_uchar)y;
    dos_uint qxy = ((dos_uint)(dos_uchar)y << 8) | (dos_uchar)x;
    dos_uint qwh = ((dos_uint)(dos_uchar)h << 8) | (dos_uchar)w;
    dos_uint qfar = (dos_uint)(qxy + qwh);
    dos_uchar x2 = (dos_uchar)((qfar & 0xFF) - 1);
    dos_uchar y2 = (dos_uchar)(((qfar >> 8) & 0xFF) - 1);
    dos_uchar count = g2f30[0];
    dos_uchar *rp = g2f30 + 1;
    dos_uchar k;

    for (k = 0; k < count; k++, rp += 5) {
        dos_uchar id = rp[0];
        dos_uchar rx = rp[1], ry = rp[2], rw = rp[3], rh = rp[4];
        dos_uint rxy, rwh, rfar;
        dos_uchar cx2, cy2;

        if (x2 < rx) continue;
        if (y2 < ry) continue;

        rxy = ((dos_uint)ry << 8) | rx;
        rwh = ((dos_uint)rh << 8) | rw;
        rfar = (dos_uint)(rxy + rwh);
        cx2 = (dos_uchar)(rfar & 0xFF);
        cy2 = (dos_uchar)((rfar >> 8) & 0xFF);

        if (cx2 <= x1) continue;
        if (cy2 <= y1) continue;
        return id;
    }
    return 0;
}

/* ---- F_D8F0 (original code at 0xD8F0) ---- */
/* F_D8F0 -- rebuild the nine-slot panel from the record at gc5da+8: for
 * each of the 9 bytes at gc5da+8..+10 that isn't -1 (an active slot
 * index), recompute that slot's level field, reload its voice pair, and
 * mark it enabled in gc6ab.
 *
 * generator type issue: g3044 is historically `struct U g3044[]`
 * (0x38-byte records, `int a` as the first field -- see the historical
 * file's struct U and its "mov dx,0x38; mul dx" comment) but
 * game_data.h emits it as a raw `uint8_t *g3044` macro alias into the
 * shared sound_instrument_region union (state-map.md rule F).  Adapted
 * below by computing the same 0x38-byte stride by hand and writing the
 * 16-bit `a` field with dos_wr16 rather than reinterpreting the byte
 * pointer through a wider (int *) type (dos_types.h: "never reinterpret
 * buffers through wider pointer types"). */
dos_int record_panel_rebuild(void)
{
    dos_char *p = gc5da + 8;   /* bp-04, historical `char far *p` */
    dos_int i;                  /* si, historical register int i */

    memset(gc6ab, 0, 9);
    movmem(gc5da + 0x11, gca62, 9);
    for (i = 0; i < 9; i++, p++) {
        if (*p != -1) {
            dos_uint slot = (dos_uint)*p * 0x38;
            dos_int a = (dos_int)(0x3f - gc5da[i + 0x1a] * 9);
            dos_wr16(g3044 + slot, (uint16_t)a);
            voice_slot_load_pair(i, g301a + slot);
            gc6ab[i] = 1;
        }
    }
    return 0;
}
