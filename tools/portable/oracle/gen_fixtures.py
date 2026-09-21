"""Generate portable/tests/fixtures/{gfx_cases,decode_cases,resource_golden_dos}.json
from real DOS runs of build/oracle/ORACLE.EXE.

Each fixture file is produced by ONE batched ORACLE.IN script (many
commands back to back) run through ONE MS-DOS Player invocation, to keep
process-launch overhead bounded; ORACLE.OUT is then parsed back into one
result record per case, in script order.

Usage: python tools/portable/oracle/gen_fixtures.py [--dosbox] [--skip-resource]
"""
import argparse
import hashlib
import json
import struct
import sys
from pathlib import Path

ORACLE_DIR = Path(__file__).resolve().parent
ROOT = ORACLE_DIR.parents[2]
sys.path.insert(0, str(ROOT / 'tools'))

from reconstruct import read_json                            # noqa: E402
from dos_runner import resolve_runner                         # noqa: E402
import build_oracle                                           # noqa: E402

FIXTURES = ROOT / 'portable/tests/fixtures'

OPS = {
    'bar': 0, 'vline': 1, 'clear_rect': 2, 'fill_rect': 3, 'save_rect': 4,
    'restore_rect': 5, 'wipe_rect': 6, 'copy_rect_flip_v': 7, 'copy_rect_flip_h': 8,
    'copy_rect_flip_hv': 9, 'copy_rect_split': 10, 'copy_rect_split_flip_v': 11,
    'draw_char': 12, 'blit_bitmap': 13, 'copy_rect': 14, 'blit_image': 15,
    'set_pixel': 16, 'get_pixel': 17,
}

FB_ROWS = 488
FB_STRIDE = 0xA0        # display_mode 4 (EGA/packed 4bpp): 160 bytes/row
VGA_FB_STRIDE = 0x140   # display_mode 5 (VGA/AE000_002, 8bpp): 320 bytes/row


# ---------------------------------------------------------------------------
# Script builder / output parser
# ---------------------------------------------------------------------------

class Script:
    """Builds an ORACLE.IN command stream and also records each logical
    "unit" (everything since the previous unit boundary -- a DUMP, a
    DECODE or a RESOURCE command, whichever ends it) as its own byte
    segment.  Long-running MS-DOS Player sessions empirically became
    unreliable somewhere past a few dozen cumulative gfx cases in one
    process (see tools/portable/oracle/README.md, "batch size"); the
    segments let callers replay an arbitrary subset of units across
    several separate ORACLE.EXE invocations instead of one giant run,
    without duplicating the case-generation logic."""

    def __init__(self):
        self.buf = bytearray(b'ORC1')
        self.segments = []
        self._mark = len(self.buf)

    def _cut(self):
        self.segments.append(bytes(self.buf[self._mark:len(self.buf)]))
        self._mark = len(self.buf)

    def seed_fb(self, seed):
        self.buf += bytes([0x01]) + struct.pack('<I', seed & 0xFFFFFFFF)

    def set_state(self, result=0, gbc=0, g94=0, g96=487, g98=0, g9a=79):
        self.buf += bytes([0x02]) + struct.pack('<6h', result, gbc, g94, g96, g98, g9a)

    def set_font(self, c0de, c0e2, c0e4, c0e6, line_height, blob):
        self.buf += bytes([0x03]) + struct.pack('<5h', c0de, c0e2, c0e4, c0e6, line_height)
        self.buf += struct.pack('<H', len(blob)) + blob
        self._cut()  # font state must be replayed at the front of every chunk

    def call(self, op_name, args, blob=b''):
        args = list(args) + [0] * (6 - len(args))
        assert len(args) == 6, args
        self.buf += bytes([0x04, OPS[op_name]]) + struct.pack('<6h', *args)
        self.buf += struct.pack('<H', len(blob)) + blob

    def dump(self):
        self.buf += bytes([0x05])
        self._cut()

    def decode(self, kind, param, blob):
        self.buf += bytes([0x06, kind]) + struct.pack('<H', param) + struct.pack('<H', len(blob)) + blob
        self._cut()

    def resource(self, dir_, idx):
        self.buf += bytes([0x07, dir_]) + struct.pack('<I', idx)
        self._cut()

    def end(self):
        self.buf += bytes([0xFF])

    def bytes(self):
        return bytes(self.buf)


class Reader:
    def __init__(self, data):
        self.data = data
        self.at = 0

    def _take(self, n):
        v = self.data[self.at:self.at + n]
        if len(v) != n:
            raise ValueError(f'ORACLE.OUT truncated at {self.at}, wanted {n} bytes')
        self.at += n
        return v

    def dump(self, stride=FB_STRIDE):
        tag = self._take(1)[0]
        if tag != 0xD0:
            raise ValueError(f'expected DUMP tag 0xD0 at {self.at - 1}, got 0x{tag:02X}')
        fb = self._take(stride * FB_ROWS)
        dlen = struct.unpack('<H', self._take(2))[0]
        dirty = self._take(dlen)
        ret = struct.unpack('<h', self._take(2))[0]
        save_len = struct.unpack('<H', self._take(2))[0]
        save = self._take(save_len)
        return {'fb': fb, 'dirty': dirty, 'ret': ret, 'save': save}

    def decode(self):
        tag = self._take(1)[0]
        if tag != 0xD2:
            raise ValueError(f'expected DECODE tag 0xD2 at {self.at - 1}, got 0x{tag:02X}')
        ret = struct.unpack('<h', self._take(2))[0]
        outlen = struct.unpack('<H', self._take(2))[0]
        data = self._take(outlen)
        return {'ret': ret, 'data': data}

    def resource(self):
        tag = self._take(1)[0]
        if tag != 0xD1:
            raise ValueError(f'expected RESOURCE tag 0xD1 at {self.at - 1}, got 0x{tag:02X}')
        s = struct.unpack('<h', self._take(2))[0]
        rtype = self._take(1)[0]
        outlen = struct.unpack('<H', self._take(2))[0]
        data = self._take(outlen)
        return {'s': s, 'type': rtype, 'data': data}

    def eof(self):
        return self.at >= len(self.data)


def run_oracle(runner, script_bytes, label, exe_name='ORACLE.EXE'):
    work = ROOT / 'build/oracle' / f'run-{label}'
    if work.exists():
        import shutil
        shutil.rmtree(work)
    work.mkdir(parents=True)
    import shutil
    shutil.copyfile(ROOT / 'build/oracle' / exe_name, work / exe_name)
    for name in ('AE000.DAT', 'AE001.DAT'):
        src = ROOT / 'assets' / name
        if src.exists():
            shutil.copyfile(src, work / name)
    (work / 'ORACLE.IN').write_bytes(script_bytes)
    result, command, output = runner.run(work / exe_name, [], work, timeout=300)
    (work / 'RUN.LOG').write_bytes(output)
    if result.returncode:
        raise SystemExit(f'{exe_name} ({label}) failed, exit {result.returncode}; see {work / "RUN.LOG"}\n'
                         f'--- tail ---\n{output[-2000:].decode("latin1", "replace")}')
    out = (work / 'ORACLE.OUT').read_bytes()
    return Reader(out)


def sha256(data):
    return hashlib.sha256(data).hexdigest()


def rows_of_interest(spans, cap=8):
    """spans: iterable of (y, h) -- returns <=cap sorted row indices around them."""
    rows = set()
    for y, h in spans:
        for r in range(max(0, y - 1), min(FB_ROWS, y + max(h, 1) + 1)):
            rows.add(r)
    rows = sorted(rows)
    if len(rows) > cap:
        rows = rows[:cap]
    return rows


def fb_row(fb, row, stride=FB_STRIDE):
    return fb[row * stride:(row + 1) * stride]


def finish_case(name, seed, state, calls, dump, touch_spans, font=None, mode=4, stride=FB_STRIDE):
    rows = rows_of_interest(touch_spans)
    case = {
        'name': name,
        'mode': mode,
        'seed': seed,
        'state': state,
        'calls': calls,
        'expect': {
            'fb_sha256': sha256(dump['fb']),
            'fb_rows_touched': {str(r): fb_row(dump['fb'], r, stride).hex() for r in rows},
            'dirty_hex': dump['dirty'].hex(),
            'ret': dump['ret'],
            'save_hex': dump['save'].hex() if dump['save'] else None,
        },
    }
    if font is not None:
        case['font'] = font
    return case


# ---------------------------------------------------------------------------
# gfx_cases.json
# ---------------------------------------------------------------------------

def make_bitmap_blob(bytes_per_row, rows, data):
    assert 0 < bytes_per_row <= 255 and 0 < rows <= 255
    assert len(data) == bytes_per_row * rows
    return b'\x5A' * 32 + bytes([bytes_per_row, rows]) + data


def make_image_blob(bytes_per_row, rows, data):
    assert len(data) == bytes_per_row * rows
    return bytes([bytes_per_row, rows]) + data


def make_bitmap_blob_vga(bytes_per_row, rows, data):
    """blit_bitmap/copy_rect blob for the VGA runtime: same 32-byte-header +
    word(bpr,rows) + packed-4bpp-data layout as make_bitmap_blob (the
    resource itself is shared between EGA and VGA rendering; `data` is
    still packed 4bpp, 2 pixels/byte), but the header's two halves are now
    meaningful: bytes 0..15 are the EGA color table (unused by the VGA
    code, given distinct values so a EGA/VGA mixup would produce visibly
    wrong output rather than accidentally matching), bytes 16..31 are the
    VGA table (`xlat`-indexed: nibble value -> 8bpp color, 0xA0+i so it is
    trivially distinguishable from real pixel data)."""
    assert 0 < bytes_per_row <= 255 and 0 < rows <= 255
    assert len(data) == bytes_per_row * rows
    ega_table = bytes((0x50 + i) & 0xFF for i in range(16))
    vga_table = bytes((0xA0 + i) & 0xFF for i in range(16))
    return ega_table + vga_table + bytes([bytes_per_row, rows]) + data


def nibble_data(bpr, rows, with_zero=False):
    """Packed-4bpp source pixel bytes for copy_rect-family tests (shared by
    both EGA and VGA case generators -- the packed-nibble *source* format
    is identical; only how the destination gets painted differs)."""
    out = bytearray()
    for i in range(bpr * rows):
        v = (i * 0x27 + 3) & 0xFF
        if with_zero and i % 5 == 0:
            v &= 0xF0  # low nibble transparent
        if with_zero and i % 7 == 0:
            v &= 0x0F  # high nibble transparent
        out.append(v)
    return bytes(out)


FONT_WIDTHS = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 16, 20]
FONT_LINE_HEIGHT = 8
FONT_PER_GLYPH_BYTES = 32  # 4 bytes/row (covers width<=32) * 8 rows -- generous, glyphs never collide


def build_font_blob():
    """One glyph per width in FONT_WIDTHS (codes 1..len(FONT_WIDTHS)),
    dialog_line_height=FONT_LINE_HEIGHT rows each.

    Layout inside the font block (all offsets relative to the blob's own
    start -- this whole blob is what gc0e0 (a plain *segment* word, not a
    far pointer) points at; gc0de/gc0e2/gc0e4/gc0e6 below are exactly the
    offsets recorded in each case's "font" fixture field, see gfx.h):
      0x10..0x1F  width table  (byte per glyph code, index = glyph code)
      0x20..0x2F  data-offset low byte table
      0x30..0x3F  data-offset high byte table
      0x40..      glyph bitmap rows, FONT_PER_GLYPH_BYTES reserved per glyph
    """
    n = len(FONT_WIDTHS)
    block = bytearray(0x40 + n * FONT_PER_GLYPH_BYTES)
    for i, width in enumerate(FONT_WIDTHS):
        glyph = i + 1
        off = i * FONT_PER_GLYPH_BYTES
        block[0x10 + glyph] = width
        block[0x20 + glyph] = off & 0xFF
        block[0x30 + glyph] = (off >> 8) & 0xFF
        bpr = (width + 7) // 8
        for row in range(FONT_LINE_HEIGHT):
            base = 0x40 + off + row * bpr
            # deterministic non-trivial bit pattern per glyph/row
            pattern = (0xB6 ^ (glyph * 17 + row * 3)) & 0xFF
            for b in range(bpr):
                block[base + b] = (pattern + b * 0x11) & 0xFF
    return bytes(block), dict(c0de=0x40, c0e2=0x30, c0e4=0x10, c0e6=0x20, line_height=FONT_LINE_HEIGHT)


def gen_gfx_cases(script, seed_counter):
    cases_meta = []  # (name, seed, state, calls, touch_spans)

    def add(name, calls, state=None, touch_spans=None, seed=None, font=None):
        nonlocal seed_counter
        if seed is None:
            seed_counter += 1
            seed = seed_counter * 2654435761 & 0xFFFFFFFF
        state = state or dict(result=0x00, gbc=0, g94=0, g96=487, g98=0, g9a=79)
        script.seed_fb(seed)
        script.set_state(**state)
        for c in calls:
            script.call(c['op'], c['args'], c.get('blob', b''))
        script.dump()
        cases_meta.append((name, seed, state, calls, touch_spans or [(0, 4)], font))

    # ---- bar: even/odd x, widths 1..9, near byte boundaries ----
    for i, (x, n, y) in enumerate([(0, 1, 3), (1, 1, 3), (2, 4, 10), (3, 5, 10),
                                    (0, 9, 20), (79 * 2 - 1, 1, 5), (158, 2, 5), (10, 3, 199)]):
        add(f'bar_x{x}_n{n}_y{y}', [{'op': 'bar', 'args': [x, y, n]}],
            state=dict(result=0x1A, gbc=0, g94=0, g96=487, g98=0, g9a=79),
            touch_spans=[(y, 1)])

    # ---- vline: even/odd x, heights 1..9 ----
    for x, n, y in [(0, 1, 3), (1, 1, 3), (4, 5, 10), (5, 9, 10), (200, 3, 5), (201, 4, 202)]:
        add(f'vline_x{x}_n{n}_y{y}', [{'op': 'vline', 'args': [x, y, n]}],
            state=dict(result=0x2B, gbc=0, g94=0, g96=487, g98=0, g9a=79),
            touch_spans=[(y, n)])

    # ---- clear_rect / fill_rect: widths/heights crossing byte boundaries ----
    for x, y, w, h in [(0, 0, 1, 1), (1, 0, 1, 1), (0, 5, 9, 3), (3, 5, 8, 4),
                        (2, 198, 4, 4), (0, 480, 10, 8)]:
        add(f'clear_x{x}_y{y}_w{w}_h{h}', [{'op': 'clear_rect', 'args': [x, y, w, h]}],
            state=dict(result=0x37, gbc=0, g94=0, g96=487, g98=0, g9a=79),
            touch_spans=[(y, h)])
        add(f'fill_x{x}_y{y}_w{w}_h{h}', [{'op': 'fill_rect', 'args': [x, y, w, h]}],
            state=dict(result=0x37, gbc=0, g94=0, g96=487, g98=0, g9a=79),
            touch_spans=[(y, h)])

    # ---- save_rect: even/odd x, several sizes ----
    for x, y, w, h in [(0, 10, 8, 3), (1, 10, 8, 3), (4, 40, 12, 5), (0, 199, 4, 2)]:
        add(f'save_x{x}_y{y}_w{w}_h{h}', [{'op': 'save_rect', 'args': [x, y, w, h]}],
            touch_spans=[(y, h)])

    # ---- restore_rect: hand-authored buffers, even/odd x ----
    for x, y, bpr, rows in [(0, 60, 4, 3), (1, 60, 4, 3), (2, 300, 6, 4)]:
        data = bytes(((i * 0x11 + 7) & 0xFF) for i in range(bpr * rows))
        buf = struct.pack('<HH', bpr, rows) + data
        add(f'restore_x{x}_y{y}_bpr{bpr}_rows{rows}',
            [{'op': 'restore_rect', 'args': [x, y], 'blob': buf}],
            touch_spans=[(y, rows)])

    # ---- wipe_rect: y<200 vs y>=200 (dirty gating), gbc 0/1, even/odd sx/dx ----
    for gbc in (0, 1):
        for sx, sy, w, h, dx, dy in [(0, 10, 8, 3, 20, 15), (1, 10, 8, 3, 21, 15),
                                      (0, 190, 6, 4, 5, 197), (0, 300, 6, 4, 5, 400)]:
            add(f'wipe_gbc{gbc}_sx{sx}_sy{sy}_dx{dx}_dy{dy}',
                [{'op': 'wipe_rect', 'args': [sx, sy, w, h, dx, dy]}],
                state=dict(result=0, gbc=gbc, g94=0, g96=487, g98=0, g9a=79),
                touch_spans=[(sy, h), (dy, h)])

    # ---- copy_rect_flip_v / flip_h / flip_hv / split / split_flip_v ----
    for opname in ('copy_rect_flip_v', 'copy_rect_flip_h', 'copy_rect_flip_hv',
                   'copy_rect_split', 'copy_rect_split_flip_v'):
        for sx, sy, w, h, dx, dy in [(0, 5, 8, 4, 30, 5), (1, 5, 8, 4, 31, 5), (0, 5, 20, 6, 0, 60)]:
            add(f'{opname}_sx{sx}_sy{sy}_dx{dx}_dy{dy}',
                [{'op': opname, 'args': [sx, sy, w, h, dx, dy]}],
                state=dict(result=0, gbc=1, g94=0, g96=487, g98=0, g9a=79),
                touch_spans=[(sy, h), (dy, h)])

    # ---- draw_char: widths 1..12,16,20, even/odd x (font set up once, up
    # front, but the exact same font info is recorded on every draw_char
    # case below so each case is independently replayable without needing
    # the SET_FONT command that happened to precede it in this script) ----
    font_blob, font_state = build_font_blob()
    script.set_font(font_state['c0de'], font_state['c0e2'], font_state['c0e4'],
                    font_state['c0e6'], font_state['line_height'], font_blob)
    font_segment_index = len(script.segments) - 1  # set_font() just cut its own segment
    font_fixture = {
        'blob_hex': font_blob.hex(),
        'gc0de': font_state['c0de'], 'gc0e2': font_state['c0e2'],
        'gc0e4': font_state['c0e4'], 'gc0e6': font_state['c0e6'],
        'line_height': font_state['line_height'],
    }
    for i, width in enumerate(FONT_WIDTHS):
        glyph = i + 1
        for x in (40, 41):
            add(f'draw_char_g{glyph}_w{width}_x{x}',
                [{'op': 'draw_char', 'args': [x, 50, glyph]}],
                state=dict(result=0x0F, gbc=0, g94=0, g96=487, g98=0, g9a=79),
                touch_spans=[(50, FONT_LINE_HEIGHT)], font=font_fixture)

    # ---- blit_bitmap: 32-byte hdr + (bpr,rows) + rows, even/odd x ----
    for x, y, bpr, rows in [(0, 70, 4, 3), (1, 70, 4, 3), (2, 250, 6, 5)]:
        data = bytes(((i * 0x13 + 5) & 0xFF) for i in range(bpr * rows))
        add(f'blit_bitmap_x{x}_y{y}_bpr{bpr}_rows{rows}',
            [{'op': 'blit_bitmap', 'args': [x, y], 'blob': make_bitmap_blob(bpr, rows, data)}],
            touch_spans=[(y, rows)])

    # ---- copy_rect: flip 0/1, transparent (zero) nibbles, clipping ----

    for x, y, bpr, rows, flip, g94_, g96_, g98_, g9a_, label in [
        (10, 20, 4, 4, 0, 0, 487, 0, 79, 'noclip'),
        (10, 20, 4, 4, 1, 0, 487, 0, 79, 'noclip_flip'),
        (10, 20, 4, 4, 0, 25, 30, 0, 79, 'row_partial_clip'),
        (10, 20, 4, 4, 0, 100, 110, 0, 79, 'row_full_clip'),
        (10, 20, 4, 4, 0, 0, 487, 3, 4, 'col_partial_clip'),
        # byte-columns only run 0..79 (160 bytes/row); out-of-domain clip words
        # (e.g. 90..95) hit an unguarded negative-count path in
        # runtime_copy_primitive, so "fully clipped" stays inside the valid
        # domain: target byte-columns 5..8 (x=10, bpr=4), clip window 0..2.
        (10, 20, 4, 4, 0, 0, 487, 0, 2, 'col_full_clip'),
    ]:
        data = nibble_data(bpr, rows, with_zero=True)
        add(f'copy_rect_{label}_flip{flip}',
            [{'op': 'copy_rect', 'args': [x, y, flip], 'blob': make_bitmap_blob(bpr, rows, data)}],
            state=dict(result=0, gbc=1, g94=g94_, g96=g96_, g98=g98_, g9a=g9a_),
            touch_spans=[(y, rows)])

    # ---- blit_image: see README.md "Known issue: gfx_blit_image" for the
    # full root-cause writeup.  Short version: runtime_f03cf (blit_image)
    # processes a source byte's bits in groups of 16 ("bx = bytes_per_row*4;
    # ...; sub bx,0x10; jg $-67h"); whenever fewer than 16 remain (every
    # bytes_per_row not a multiple of 4, which is the common case for real
    # image widths) it falls into "jmp word ptr cs:[bx+
    # runtime_even_pixel_dispatch-...]" / ...odd_pixel_dispatch, but that
    # table (defined once, shared with runtime_f03c6/draw_char) holds
    # draw_char's rt_14xx targets, not blit_image's own rt_18xx/rt_15xx
    # targets -- which sit a few dozen bytes further down, unused, in a
    # table the file itself labels "Unreferenced translated copy of
    # runtime_even_pixel_dispatch (+0424h)".  Confirmed empirically:
    # bytes_per_row in {4, 8, 12, 16} (no remainder, the dispatch table is
    # never reached) all render correctly; every other width from 1..9
    # tried crashes ORACLE.EXE.  Only the working widths are exercised
    # here; x parity is irrelevant (runtime_f03cf has no odd-x branch --
    # `shr bx,1; add di,bx` with nothing testing the shift's carry flag,
    # unlike bar/vline/etc).
    for x, y, bpr, rows in [(0, 90, 4, 3), (5, 90, 4, 3), (0, 250, 8, 5),
                            (2, 350, 12, 4), (0, 400, 16, 6)]:
        data = bytes(((i * 0x5B + 0x3C) & 0xFF) for i in range(bpr * rows))
        add(f'blit_image_x{x}_y{y}_bpr{bpr}_rows{rows}',
            [{'op': 'blit_image', 'args': [x, y], 'blob': make_image_blob(bpr, rows, data)}],
            state=dict(result=0x21, gbc=0, g94=0, g96=487, g98=0, g9a=79),
            touch_spans=[(y, rows)])

    # ---- set_pixel / get_pixel: even/odd x ----
    for x, y in [(0, 12), (1, 12), (50, 300), (51, 300)]:
        add(f'set_pixel_x{x}_y{y}', [{'op': 'set_pixel', 'args': [x, y]}],
            state=dict(result=0x09, gbc=0, g94=0, g96=487, g98=0, g9a=79),
            touch_spans=[(y, 1)])
    for x, y in [(0, 12), (1, 12), (50, 300), (51, 300)]:
        add(f'get_pixel_x{x}_y{y}', [{'op': 'bar', 'args': [x, y, 1]}, {'op': 'get_pixel', 'args': [x, y]}],
            state=dict(result=0x0C, gbc=0, g94=0, g96=487, g98=0, g9a=79),
            touch_spans=[(y, 1)])

    return cases_meta, font_segment_index


# MS-DOS Player sessions empirically became unreliable (Turbo C runtime's
# "Abnormal program termination") somewhere past several dozen cumulative
# gfx cases in one process -- see README.md, "batch size". Cases are
# generated once (in Python, no DOS involved) and then replayed through
# ORACLE.EXE in bounded chunks; every chunk gets a copy of the font-setup
# unit up front so draw_char cases still see valid font state.
GFX_CHUNK_SIZE = 25


def run_gfx(runner):
    script = Script()
    cases_meta, font_segment_index = gen_gfx_cases(script, seed_counter=1000)
    case_segments = [seg for i, seg in enumerate(script.segments) if i != font_segment_index]
    font_segment = script.segments[font_segment_index]
    assert len(case_segments) == len(cases_meta)

    cases = []
    for chunk_no, start in enumerate(range(0, len(cases_meta), GFX_CHUNK_SIZE)):
        chunk_meta = cases_meta[start:start + GFX_CHUNK_SIZE]
        chunk_segs = case_segments[start:start + GFX_CHUNK_SIZE]
        script_bytes = b'ORC1' + font_segment + b''.join(chunk_segs) + bytes([0xFF])
        reader = run_oracle(runner, script_bytes, f'gfx-{chunk_no:02d}')
        for name, seed, state, calls, spans, font in chunk_meta:
            dump = reader.dump()
            json_calls = [{'op': c['op'], 'args': c['args'], **({'blob_hex': c['blob'].hex()} if c.get('blob') else {})}
                         for c in calls]
            cases.append(finish_case(name, seed, state, json_calls, dump, spans, font=font))
        if not reader.eof():
            raise SystemExit(f'gfx chunk {chunk_no}: {len(reader.data) - reader.at} unread trailing bytes in ORACLE.OUT')
    return cases


# ---------------------------------------------------------------------------
# gfx_cases.json (VGA / display_mode 5 / ORACLEV.EXE) -- see
# docs/portable/reference/AE000_002-vga-runtime.lst.  Word counts/argument
# order for every primitive are identical to the EGA runtime; the only
# semantic difference is that x/y/w/h/n are plain PIXEL (=byte, 8bpp)
# values instead of packed-nibble byte-columns, EXCEPT gfx_copy_rect's
# clip comparisons against g98/g9a and the dirty-queue x-records, which
# the VGA machine code still computes in the same packed (x/2) unit as
# EGA (confirmed: `sar bx,1` before the g98h/g9ah compares, `shr ax,1`
# before every dirty-queue x/w stosw).  gfx_box is skipped (present/VRAM,
# out of scope, same as the EGA harness).
# ---------------------------------------------------------------------------

def gen_vga_cases(script, seed_counter):
    cases_meta = []

    def add(name, calls, state=None, touch_spans=None, seed=None, font=None):
        nonlocal seed_counter
        if seed is None:
            seed_counter += 1
            seed = seed_counter * 2654435761 & 0xFFFFFFFF
        # g9a=159: gfx_copy_rect's only clip in play here compares against
        # x/2 (packed units); the VGA canvas is 320 px wide -> 160 packed
        # columns, valid range 0..159 (matches g9a=79 for EGA's 160-byte,
        # 80-column canvas).
        state = state or dict(result=0x00, gbc=0, g94=0, g96=487, g98=0, g9a=159)
        script.seed_fb(seed)
        script.set_state(**state)
        for c in calls:
            script.call(c['op'], c['args'], c.get('blob', b''))
        script.dump()
        cases_meta.append((name, seed, state, calls, touch_spans or [(0, 4)], font))

    # ---- bar: widths 1..9, x near both edges of the 320px-wide canvas ----
    for x, n, y in [(0, 1, 3), (1, 1, 3), (2, 4, 10), (3, 5, 10),
                    (0, 9, 20), (317, 1, 5), (300, 2, 5), (150, 3, 199)]:
        add(f'bar_x{x}_n{n}_y{y}', [{'op': 'bar', 'args': [x, y, n]}],
            state=dict(result=0x1A, gbc=0, g94=0, g96=487, g98=0, g9a=159),
            touch_spans=[(y, 1)])

    # ---- vline: heights 1..9 ----
    for x, n, y in [(0, 1, 3), (200, 1, 3), (4, 5, 10), (5, 9, 10), (200, 3, 5), (201, 4, 202)]:
        add(f'vline_x{x}_n{n}_y{y}', [{'op': 'vline', 'args': [x, y, n]}],
            state=dict(result=0x2B, gbc=0, g94=0, g96=487, g98=0, g9a=159),
            touch_spans=[(y, n)])

    # ---- clear_rect / fill_rect ----
    for x, y, w, h in [(0, 0, 1, 1), (5, 5, 9, 3), (2, 198, 16, 4), (0, 480, 20, 8)]:
        add(f'clear_x{x}_y{y}_w{w}_h{h}', [{'op': 'clear_rect', 'args': [x, y, w, h]}],
            state=dict(result=0x37, gbc=0, g94=0, g96=487, g98=0, g9a=159),
            touch_spans=[(y, h)])
        add(f'fill_x{x}_y{y}_w{w}_h{h}', [{'op': 'fill_rect', 'args': [x, y, w, h]}],
            state=dict(result=0x37, gbc=0, g94=0, g96=487, g98=0, g9a=159),
            touch_spans=[(y, h)])

    # ---- save_rect / restore_rect (header is now plain w,h in pixels) ----
    for x, y, w, h in [(0, 10, 8, 3), (4, 40, 12, 5), (0, 199, 5, 2), (150, 300, 9, 4)]:
        add(f'save_x{x}_y{y}_w{w}_h{h}', [{'op': 'save_rect', 'args': [x, y, w, h]}],
            touch_spans=[(y, h)])

    for x, y, w, rows in [(0, 60, 4, 3), (10, 60, 7, 3), (2, 300, 6, 4)]:
        data = bytes(((i * 0x11 + 7) & 0xFF) for i in range(w * rows))
        buf = struct.pack('<HH', w, rows) + data
        add(f'restore_x{x}_y{y}_w{w}_rows{rows}',
            [{'op': 'restore_rect', 'args': [x, y], 'blob': buf}],
            touch_spans=[(y, rows)])

    # ---- wipe_rect: gbc 0/1, y<200 vs y>=200 (dirty gating) ----
    for gbc in (0, 1):
        for sx, sy, w, h, dx, dy in [(0, 10, 8, 3, 20, 15), (5, 10, 9, 3, 21, 15),
                                      (0, 190, 6, 4, 5, 197), (0, 300, 6, 4, 5, 400)]:
            add(f'wipe_gbc{gbc}_sx{sx}_sy{sy}_dx{dx}_dy{dy}',
                [{'op': 'wipe_rect', 'args': [sx, sy, w, h, dx, dy]}],
                state=dict(result=0, gbc=gbc, g94=0, g96=487, g98=0, g9a=159),
                touch_spans=[(sy, h), (dy, h)])

    # ---- copy_rect_flip_v / flip_h / flip_hv / split / split_flip_v:
    # even AND odd sizes ----
    for opname in ('copy_rect_flip_v', 'copy_rect_flip_h', 'copy_rect_flip_hv',
                   'copy_rect_split', 'copy_rect_split_flip_v'):
        for sx, sy, w, h, dx, dy in [(0, 5, 8, 4, 30, 5), (1, 5, 7, 5, 31, 5), (0, 5, 20, 6, 0, 60)]:
            add(f'{opname}_sx{sx}_sy{sy}_w{w}_h{h}_dx{dx}_dy{dy}',
                [{'op': opname, 'args': [sx, sy, w, h, dx, dy]}],
                state=dict(result=0, gbc=1, g94=0, g96=487, g98=0, g9a=159),
                touch_spans=[(sy, h), (dy, h)])

    # ---- draw_char: widths 1..12,16,20, even/odd x (same synthetic font
    # as the EGA cases -- font state/table layout is DGROUP-identical) ----
    font_blob, font_state = build_font_blob()
    script.set_font(font_state['c0de'], font_state['c0e2'], font_state['c0e4'],
                    font_state['c0e6'], font_state['line_height'], font_blob)
    font_segment_index = len(script.segments) - 1
    font_fixture = {
        'blob_hex': font_blob.hex(),
        'gc0de': font_state['c0de'], 'gc0e2': font_state['c0e2'],
        'gc0e4': font_state['c0e4'], 'gc0e6': font_state['c0e6'],
        'line_height': font_state['line_height'],
    }
    for i, width in enumerate(FONT_WIDTHS):
        glyph = i + 1
        for x in (40, 41):
            add(f'draw_char_g{glyph}_w{width}_x{x}',
                [{'op': 'draw_char', 'args': [x, 50, glyph]}],
                state=dict(result=0x0F, gbc=0, g94=0, g96=487, g98=0, g9a=159),
                touch_spans=[(50, FONT_LINE_HEIGHT)], font=font_fixture)

    # ---- blit_bitmap: packed-4bpp source, EGA table at +0 / VGA table at
    # +0x10 (see make_bitmap_blob_vga) ----
    for x, y, bpr, rows in [(0, 70, 4, 3), (5, 70, 4, 3), (2, 250, 6, 5), (0, 350, 3, 4)]:
        data = bytes(((i * 0x13 + 5) & 0xFF) for i in range(bpr * rows))
        add(f'blit_bitmap_x{x}_y{y}_bpr{bpr}_rows{rows}',
            [{'op': 'blit_bitmap', 'args': [x, y], 'blob': make_bitmap_blob_vga(bpr, rows, data)}],
            touch_spans=[(y, rows)])

    # ---- copy_rect: flip 0/1, transparent (zero) nibbles, row/column clip
    # (g98/g9a stay in packed x/2 units; unclipped default is 0..159) ----
    for x, y, bpr, rows, flip, g94_, g96_, g98_, g9a_, label in [
        (10, 20, 4, 4, 0, 0, 487, 0, 159, 'noclip'),
        (10, 20, 4, 4, 1, 0, 487, 0, 159, 'noclip_flip'),
        (10, 20, 4, 4, 0, 25, 30, 0, 159, 'row_partial_clip'),
        (10, 20, 4, 4, 0, 100, 110, 0, 159, 'row_full_clip'),
        (10, 20, 4, 4, 0, 0, 487, 3, 4, 'col_partial_clip'),
        (10, 20, 4, 4, 0, 0, 487, 0, 2, 'col_full_clip'),   # target packed-cols 5..8, clip 0..2
        (5, 400, 3, 5, 1, 0, 487, 0, 159, 'flip1_lowy'),
    ]:
        data = nibble_data(bpr, rows, with_zero=True)
        add(f'copy_rect_{label}_flip{flip}',
            [{'op': 'copy_rect', 'args': [x, y, flip], 'blob': make_bitmap_blob_vga(bpr, rows, data)}],
            state=dict(result=0, gbc=1, g94=g94_, g96=g96_, g98=g98_, g9a=g9a_),
            touch_spans=[(y, rows)])

    # ---- blit_image: n (bytes_per_row) = 1,2,3,5 -- this VGA routine has
    # no dispatch-table bug (unrolled straight-line 8-way unroll, no CS
    # jump table), so small widths are fine ----
    for x, y, bpr, rows in [(0, 90, 1, 6), (5, 90, 2, 5), (0, 250, 3, 4), (2, 350, 5, 6)]:
        data = bytes(((i * 0x5B + 0x3C) & 0xFF) for i in range(bpr * rows))
        add(f'blit_image_x{x}_y{y}_bpr{bpr}_rows{rows}',
            [{'op': 'blit_image', 'args': [x, y], 'blob': make_image_blob(bpr, rows, data)}],
            state=dict(result=0x21, gbc=0, g94=0, g96=487, g98=0, g9a=159),
            touch_spans=[(y, rows)])

    # ---- set_pixel / get_pixel ----
    for x, y in [(0, 12), (1, 12), (150, 300), (319, 300)]:
        add(f'set_pixel_x{x}_y{y}', [{'op': 'set_pixel', 'args': [x, y]}],
            state=dict(result=0x09, gbc=0, g94=0, g96=487, g98=0, g9a=159),
            touch_spans=[(y, 1)])
    for x, y in [(0, 12), (1, 12), (150, 300), (319, 300)]:
        add(f'get_pixel_x{x}_y{y}', [{'op': 'bar', 'args': [x, y, 1]}, {'op': 'get_pixel', 'args': [x, y]}],
            state=dict(result=0x0C, gbc=0, g94=0, g96=487, g98=0, g9a=159),
            touch_spans=[(y, 1)])

    return cases_meta, font_segment_index


VGA_CHUNK_SIZE = 25


def run_vga(runner):
    script = Script()
    cases_meta, font_segment_index = gen_vga_cases(script, seed_counter=5000)
    case_segments = [seg for i, seg in enumerate(script.segments) if i != font_segment_index]
    font_segment = script.segments[font_segment_index]
    assert len(case_segments) == len(cases_meta)

    cases = []
    for chunk_no, start in enumerate(range(0, len(cases_meta), VGA_CHUNK_SIZE)):
        chunk_meta = cases_meta[start:start + VGA_CHUNK_SIZE]
        chunk_segs = case_segments[start:start + VGA_CHUNK_SIZE]
        script_bytes = b'ORC1' + font_segment + b''.join(chunk_segs) + bytes([0xFF])
        reader = run_oracle(runner, script_bytes, f'vga-{chunk_no:02d}', exe_name='ORACLEV.EXE')
        for name, seed, state, calls, spans, font in chunk_meta:
            dump = reader.dump(stride=VGA_FB_STRIDE)
            json_calls = [{'op': c['op'], 'args': c['args'], **({'blob_hex': c['blob'].hex()} if c.get('blob') else {})}
                         for c in calls]
            cases.append(finish_case(name, seed, state, json_calls, dump, spans, font=font,
                                     mode=5, stride=VGA_FB_STRIDE))
        if not reader.eof():
            raise SystemExit(f'vga chunk {chunk_no}: {len(reader.data) - reader.at} unread trailing bytes in ORACLE.OUT')
    return cases


# ---------------------------------------------------------------------------
# decode_cases.json
# ---------------------------------------------------------------------------

def gen_decode_cases(script):
    cases_meta = []

    def add(name, kind, param, blob):
        script.decode(kind, param, blob)
        cases_meta.append((name, kind, param, blob))

    # rle_packbits_decode: control>0 -> copy n literal bytes; control<=0 -> repeat (1-n) times
    add('rle_all_literal', 0, 4, bytes([3, 0xAA, 0xBB, 0xCC]))
    add('rle_all_repeat', 0, 2, bytes([256 - 5 & 0xFF, 0x77]))          # n=-5 -> repeat 6 times... clipped by max_len below
    add('rle_mixed', 0, 8, bytes([2, 0x11, 0x22, (256 - 3) & 0xFF, 0x99, 1, 0x01]))
    add('rle_single_literal', 0, 2, bytes([1, 0x5A]))
    add('rle_zero_repeat_edgecase', 0, 3, bytes([0, 0x66]))             # control==0 -> repeat 1 time (1-0)

    # lz_decompress: exercised with tiny hand-built streams; exact bit layout is
    # validated by the harness itself (round-trips through the real ASM), not
    # re-derived here -- these are smoke vectors, not an independent codec proof.
    add('lz_smoke_a', 1, 6, bytes([0x06, 0x00, 0x00, 0x00, 0x00, 0x00]))
    add('lz_smoke_b', 1, 8, bytes([0x08, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00]))

    # sprite_decode_4bpp_planar: 16-byte palette (masked to low nibble) + packed pixel bytes
    palette = bytes((i | 0xF0) if i % 2 else i for i in range(16))  # high bits must be masked off
    pixels = bytes(((i * 0x1F) & 0xFF) for i in range(16))
    add('sprite_planar_basic', 2, 0, palette + pixels)

    palette2 = bytes(range(16))
    pixels2 = bytes([0x00, 0xFF, 0x0F, 0xF0] * 6)
    add('sprite_planar_zero_nibbles', 2, 0, palette2 + pixels2)

    # sprite_decode_4bpp_mode13h: 15-byte palette + packed pixel bytes
    palette15 = bytes((i * 4) & 0x3F for i in range(15))
    pixels15 = bytes(((i * 0x29 + 3) & 0xFF) for i in range(20))
    add('sprite_mode13h_basic', 3, 0, palette15 + pixels15)

    return cases_meta


def run_decode(runner):
    script = Script()
    cases_meta = gen_decode_cases(script)
    script.end()
    reader = run_oracle(runner, script.bytes(), 'decode')
    cases = []
    for name, kind, param, blob in cases_meta:
        result = reader.decode()
        cases.append({
            'name': name, 'kind': kind, 'param': param, 'blob_hex': blob.hex(),
            'expect': {'ret': result['ret'], 'data_hex': result['data'].hex(),
                      'data_sha256': sha256(result['data'])},
        })
    if not reader.eof():
        raise SystemExit(f'decode: {len(reader.data) - reader.at} unread trailing bytes in ORACLE.OUT')
    return cases


# ---------------------------------------------------------------------------
# resource_golden_dos.json
# ---------------------------------------------------------------------------

def record_counts():
    """Number of *records* per archive -- one less than the offset table's
    entry count: entry[i] is record i's start, but the table's LAST entry
    is only the end-of-data marker for the record before it, not the start
    of a record of its own (confirmed against
    portable/tests/fixtures/resource_golden.json: AE000 has 89 records,
    AE001 131, i.e. table_entries - 1 in each case, matching src/RESOURCE.C
    resource_load_record's own o1/o2 = entry[p]/entry[p+1] pairing, which
    has no defined o2 for p == table_entries - 1)."""
    import struct as _s
    counts = {}
    for dirn, fname in ((0, 'AE000.DAT'), (1, 'AE001.DAT')):
        path = ROOT / 'assets' / fname
        if not path.exists():
            return None
        data = path.read_bytes()
        n0 = _s.unpack_from('<I', data, 0)[0]
        counts[dirn] = n0 // 4 - 1
    return counts


RESOURCE_CHUNK_SIZE = 30


def run_resource(runner):
    counts = record_counts()
    if counts is None:
        print('assets/AE000.DAT or AE001.DAT missing; skipping resource_golden_dos.json')
        return None
    script = Script()
    order = []
    for dirn, count in counts.items():
        for idx in range(count):
            script.resource(dirn, idx)
            order.append((dirn, idx))
    assert len(script.segments) == len(order)

    golden = {}
    for chunk_no, start in enumerate(range(0, len(order), RESOURCE_CHUNK_SIZE)):
        chunk_order = order[start:start + RESOURCE_CHUNK_SIZE]
        chunk_segs = script.segments[start:start + RESOURCE_CHUNK_SIZE]
        script_bytes = b'ORC1' + b''.join(chunk_segs) + bytes([0xFF])
        reader = run_oracle(runner, script_bytes, f'resource-{chunk_no:02d}')
        for dirn, idx in chunk_order:
            result = reader.resource()
            key = f"AE{dirn:03d}_{idx:03d}"
            golden[key] = {'s': result['s'], 'type': result['type'], 'sha256': sha256(result['data'])}
        if not reader.eof():
            raise SystemExit(f'resource chunk {chunk_no}: {len(reader.data) - reader.at} unread trailing bytes in ORACLE.OUT')
    return golden


# ---------------------------------------------------------------------------
# main
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--dosbox', action='store_true')
    parser.add_argument('--skip-resource', action='store_true')
    parser.add_argument('--skip-build', action='store_true')
    args = parser.parse_args()

    lock = read_json(ROOT / 'layout/toolchain.json')
    runner = resolve_runner(lock, backend='dosbox' if args.dosbox else None)

    if not args.skip_build or not (ROOT / 'build/oracle/ORACLE.EXE').exists():
        build_oracle.build(runner=runner)
    if not args.skip_build or not (ROOT / 'build/oracle/ORACLEV.EXE').exists():
        build_oracle.build_vga(runner=runner)

    FIXTURES.mkdir(parents=True, exist_ok=True)

    gfx_cases = run_gfx(runner)
    vga_cases = run_vga(runner)
    all_gfx_cases = gfx_cases + vga_cases
    (FIXTURES / 'gfx_cases.json').write_text(json.dumps(all_gfx_cases, indent=1) + '\n', encoding='utf-8')
    print(f'gfx_cases.json: {len(all_gfx_cases)} cases ({len(gfx_cases)} mode=4 EGA + '
         f'{len(vga_cases)} mode=5 VGA), {(FIXTURES / "gfx_cases.json").stat().st_size} bytes')

    decode_cases = gen_decode_cases_and_write(runner)

    if not args.skip_resource:
        golden = run_resource(runner)
        if golden is not None:
            (FIXTURES / 'resource_golden_dos.json').write_text(json.dumps(golden, indent=1, sort_keys=True) + '\n', encoding='utf-8')
            print(f'resource_golden_dos.json: {len(golden)} records, '
                 f'{(FIXTURES / "resource_golden_dos.json").stat().st_size} bytes')


def gen_decode_cases_and_write(runner):
    decode_cases = run_decode(runner)
    (FIXTURES / 'decode_cases.json').write_text(json.dumps(decode_cases, indent=1) + '\n', encoding='utf-8')
    print(f'decode_cases.json: {len(decode_cases)} cases, {(FIXTURES / "decode_cases.json").stat().st_size} bytes')
    return decode_cases


if __name__ == '__main__':
    main()
