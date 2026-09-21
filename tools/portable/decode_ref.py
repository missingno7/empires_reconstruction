"""Independent Python reference for asm/DECODE.ASM's sprite-side decoders.

This is a from-scratch transcription of F_6EFF (sprite_decode_4bpp_planar)
and of src/RESOURCE.C's F_6771/F_67DC (sprite_sheet_decode_sequential /
sprite_sheet_decode_indexed), written directly from the ASM/C rather than
copied from portable/resource/decode.c or archive.c, so that comparing its
output against the C port's output is a real cross-check and not the same
mistake twice.

Only the display_mode == 4 (planar, non-mode13h) path is implemented: that
is the only display_mode gen_resource_golden.py exercises.  mode13h
(F_6F4B, display_mode == 2) is out of scope here.

Compression-stage decoding (the RLE / pair-span "LZ" transforms) is NOT
duplicated here -- tools/resource_codecs.py already provides a validated,
independently-authored implementation of those (proven by byte-exact
archive round-trips); gen_resource_golden.py calls it directly.
"""
from __future__ import annotations

from typing import List


def _to_int16(x: int) -> int:
    x &= 0xFFFF
    return x - 0x10000 if x >= 0x8000 else x


def _to_uint16(x: int) -> int:
    return x & 0xFFFF


def _rd16(buf: bytearray, off: int) -> int:
    return buf[off] | (buf[off + 1] << 8)


def sprite_decode_4bpp_planar(buf: bytearray, base: int) -> None:
    """F_6EFF: in-place planar 4bpp sprite/tile expansion at buf[base:].

    8 header words (16 bytes) &= 0x0F0F and double as two xlat tables; 16
    more header bytes are skipped; width/height bytes follow; then w*h
    packed pixel bytes, each split into low/high nibbles and remapped
    through the table, in place.
    """
    table: List[int] = [0] * 16
    for i in range(8):
        word = (_rd16(buf, base + 2 * i)) & 0x0F0F
        buf[base + 2 * i] = word & 0xFF
        buf[base + 2 * i + 1] = (word >> 8) & 0xFF
        table[2 * i] = buf[base + 2 * i]
        table[2 * i + 1] = buf[base + 2 * i + 1]

    w = buf[base + 32]
    h = buf[base + 33]
    count = (w * h) & 0xFFFF
    p = base + 34

    for k in range(count):
        b = buf[p + k]
        t_lo = table[b & 0x0F]
        t_hi = table[(b >> 4) & 0x0F]
        buf[p + k] = (t_hi << 4) | t_lo


def sprite_sheet_decode_sequential(buf: bytearray, base: int, n: int) -> None:
    """F_6771: walk a run of back-to-back sprite records starting at
    buf[base:base+n), decoding each in place until a non-sprite marker byte
    or n bytes are consumed.  The stride `q[34]*q[35]+36` is 16-bit signed
    `int` arithmetic in the historical C (two unsigned chars multiplied,
    truncated to 16 bits, then +36), reproduced explicitly here.
    """
    i = 0
    while i < n:
        q = base + i
        if buf[q] != 0x47:
            break
        sprite_decode_4bpp_planar(buf, q + 2)
        step = _to_int16(_to_int16(buf[q + 34] * buf[q + 35]) + 36)
        i = _to_uint16(i + step)


def sprite_sheet_decode_indexed(buf: bytearray, base: int) -> None:
    """F_67DC: walk a table of little-endian 16-bit offsets at buf[base:],
    decoding each referenced sprite record in place.  `n = (t[0]>>1) - 1`
    is signed 16-bit `int` arithmetic (wraps to -1, terminating the loop,
    when t[0] < 2).
    """
    t0 = _rd16(buf, base)
    n = _to_int16((t0 >> 1) - 1)
    t_off = base
    i = 0
    while i < n:
        off = _rd16(buf, t_off)
        q = base + off
        t_off += 2
        if buf[q] == 0x47:
            sprite_decode_4bpp_planar(buf, q + 2)
        i += 1


def decode_sprite_dispatch(buf: bytearray, gc0cb: int, display_mode: int = 4) -> None:
    """src/RESOURCE.C resource_load_record's type dispatch, display_mode !=
    5 only, display_mode == 4 (planar) semantics.  Mutates buf in place;
    the decoded length (`s`) is unaffected by this stage.
    """
    if display_mode == 5:
        return
    if gc0cb == 0x47:
        sprite_decode_4bpp_planar(buf, 0)
    elif gc0cb == 0:
        sprite_sheet_decode_sequential(buf, 0, len(buf))
    elif gc0cb == 1:
        sprite_sheet_decode_indexed(buf, 0)
    # else: no further dispatch, buf already holds the final decoded bytes.
