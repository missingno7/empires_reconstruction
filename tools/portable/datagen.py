#!/usr/bin/env python3
"""Portable DGROUP data generator.

Produces, from the historical reconstruction tree (read-only reference):

  * portable/generated/symbols.json   -- merged DGROUP symbol table
  * portable/generated/game_data.[ch] -- initialized DATA objects
  * portable/generated/game_state.[ch]-- zero-initialized BSS objects
  * docs/portable/state-map.md        -- human-readable symbol map

It never modifies the historical tree (src/, asm/, include/, recipes/,
layout/, tools/ other than tools/portable/).  It reuses the historical
DATA encoders (tools/exe_data.py, tools/typed_data.py, tools/pointer_records.py,
tools/sound_data.py, tools/sound_instruments.py) purely as *decoders* of the
already-frozen recipe sources; it never compiles anything and never invokes
the pinned DOS toolchain.

Ground truth for DATA bytes is assets/AEPROG.EXE itself (see "DATA image"
below); ground truth for the DATA/BSS *offset layout* is layout/manifest.json,
whose `regions` partition the file with no gaps or overlaps (see docs/layout.md).
recipes/data/game-initialized.json supplies component *format* and, through
the historical typed-data/pointer-record/sound-data sources, symbolic
structure (field names/types, pointer targets) -- but never final resolved
byte values, since those still require target placement, which this module
computes itself instead of relying on the OMF/TLINK path in
tools/production_data.py.

Run standalone: `python tools/portable/datagen.py [--out-root DIR]`
"""
from __future__ import annotations

import argparse
import bisect
import hashlib
import json
import re
import struct
import sys
from pathlib import Path

THIS_FILE = Path(__file__).resolve()
ROOT = THIS_FILE.parents[2]
TOOLS_DIR = ROOT / 'tools'
if str(TOOLS_DIR) not in sys.path:
    sys.path.insert(0, str(TOOLS_DIR))

from exe_data import encode_data  # noqa: E402
from typed_data import FORMAT as TYPED_FORMAT, compile_typed_data  # noqa: E402
from pointer_records import FORMAT as PTRREC_FORMAT, compile_records  # noqa: E402
from sound_data import FORMAT as SOUND_FORMAT, compile_sound_data  # noqa: E402
from sound_instruments import FORMAT as INSTR_FORMAT, compile_sound_instruments  # noqa: E402

# ---------------------------------------------------------------------------
# Constants (see docs/portable/architecture.md and docs/layout.md)
# ---------------------------------------------------------------------------

DATA_LEN = 0x3902          # 14594 -- initialized DGROUP DATA, DS:0000..DS:3902
BSS_LEN = 37250             # DS:3902..DS:C0(BFCD is inside this) -- src/data/GAME_BSS.json
DGROUP_LEN = DATA_LEN + BSS_LEN  # 0xCA84 -- whole DGROUP address space used below

MANIFEST_PATH = ROOT / 'layout/manifest.json'
PLAN_PATH = ROOT / 'layout/production-plan.json'
RECIPE_PATH = ROOT / 'recipes/data/game-initialized.json'
BSS_JSON_PATH = ROOT / 'src/data/GAME_BSS.json'
SYMNAMES_PATH = ROOT / 'docs/current/symbol-names.json'
STATE_OWNERSHIP_PATH = ROOT / 'tools/portable/state_ownership.json'
DATAGEN_OVERRIDES_PATH = ROOT / 'tools/portable/datagen_overrides.json'
EXE_PATH = ROOT / 'assets/AEPROG.EXE'
PORTABLE_DATA_DIR = ROOT / 'portable/data'


def read_json(path):
    return json.loads(Path(path).read_text(encoding='utf-8'))


# ---------------------------------------------------------------------------
# Step A -- DATA byte image + component map
# ---------------------------------------------------------------------------

def _load_component_source(spec):
    """Resolve a recipe component's source JSON.

    The two dac6-rgb256-v1 palettes live under raw/exe-data/ (gitignored,
    historical-recovery cache); portable/data/ carries a committed copy with
    the same filename, which is preferred so datagen.py works from a clean
    checkout of just the portable tree plus the read-only historical tree.
    """
    src = spec['source']
    name = Path(src).name
    portable_copy = PORTABLE_DATA_DIR / name
    if portable_copy.exists():
        return read_json(portable_copy)
    return read_json(ROOT / src)


def _structural_info(spec):
    """(data, refs, publics, extra) for one non-compiled-data component.

    Mirrors the per-format dispatch in tools/production_data.py::prepare_data,
    but calls the *compile_* functions directly (never the *bind_* / resolve
    path) so no symbolic pointer needs to be resolved yet -- refs come back
    with the target name and addend, unresolved.  `extra` carries whatever
    the emitter needs to reproduce field-level structure (typed-data field
    list, pointer-record rows, the sound-data/instrument document).
    """
    fmt = spec['format']
    doc = _load_component_source(spec)
    if fmt == TYPED_FORMAT:
        data, refs, publics = compile_typed_data(doc)
        extra = {'fields': doc['fields']}
    elif fmt == PTRREC_FORMAT:
        data, refs = compile_records(doc)
        publics = {spec['id']: 0}
        extra = {'records': doc['records']}
    elif fmt == SOUND_FORMAT:
        data, refs, publics = compile_sound_data(doc)
        extra = {'doc': doc}
    elif fmt == INSTR_FORMAT:
        data = compile_sound_instruments(doc)
        refs = []
        publics = {spec['id']: 0}
        extra = {'doc': doc}
    else:
        data = encode_data(doc, fmt)
        refs = []
        publics = {spec['id']: 0}
        extra = {'doc': doc}
    return data, refs, publics, extra


def build_component_map():
    """Return (image_bytes, components, notes).

    PRIMARY byte source: the recipe's own JSON sources, decoded through the
    historical tools/*.py encoders (tools/exe_data.py, typed_data.py,
    pointer_records.py, sound_data.py, sound_instruments.py) -- the same
    encoders tools/production_data.py::prepare_data uses, called here as
    pure decoders. `assets/AEPROG.EXE` is never read for bytes; it is only
    an OPTIONAL cross-check (see cross_check_against_exe()), and generation
    succeeds identically whether or not that file is present.

    `image_bytes` is a DATA_LEN-byte bytearray built by placing each
    recipe component's decoded bytes at its manifest-derived ds_offset
    (`region['start'] - (manifest['frames']['DGROUP'] + 512)`, the DS-
    offset coordinate docs/layout.md defines). manifest region boundaries
    are the placement authority (they partition the whole EXE with no gaps
    or overlaps, independently of this generator); recipe component *byte
    content* is the content authority. A component with `skip` (currently
    only DATA_00FCC2) places its post-skip bytes starting `skip` bytes into
    its region -- the skipped leading bytes are never written (nothing
    names them; see docs/portable/state-map.md's DS:0000-0x0092 note).

    Components belonging to a `compiled-data` spec (kind 'code_owned') and
    the manifest regions with no recipe component at all (kind
    'toolchain_opaque' -- Turbo C startup/CC.LIB runtime-library data, not
    game data, not used by the port) contribute NO bytes: their slice of
    `image_bytes` is left zeroed and nothing is emitted for it.

    `components` is one dict per manifest region that falls (even partially)
    inside [0, DATA_LEN), ordered by ds_offset, covering the whole range with
    no gaps or overlaps (asserted).
    """
    manifest = read_json(MANIFEST_PATH)
    recipe = read_json(RECIPE_PATH)

    base = manifest['frames']['DGROUP'] + 512
    manifest_end = base + DATA_LEN
    if manifest_end != max(r['end'] for r in manifest['regions']):
        raise ValueError(
            f'DGROUP DATA base {base:#x} + DATA_LEN {DATA_LEN:#x} != manifest end; the '
            '"DATA ends at end of the manifest region set" layout assumption documented in '
            "tools/portable/datagen.py no longer holds -- re-derive from "
            "layout/production-plan.json['data']/['data_placements']/['padding'].")

    image = bytearray(DATA_LEN)

    recipe_by_id = {c['id']: c for c in recipe['components']}
    regions_sorted = sorted(manifest['regions'], key=lambda r: r['start'])

    components = []
    gaps = []
    cursor = 0
    for region in regions_sorted:
        ds0 = region['start'] - base
        ds1 = region['end'] - base
        if ds1 <= 0 or ds0 >= DATA_LEN:
            continue
        ds0 = max(ds0, 0)
        ds1 = min(ds1, DATA_LEN)
        if ds0 != cursor:
            raise ValueError(
                f'DATA region gap/overlap before {region["id"]!r}: expected cursor '
                f'{cursor:#x}, region starts at {ds0:#x}')
        cursor = ds1
        length = ds1 - ds0
        spec = recipe_by_id.get(region['id'])
        entry = {
            'id': region['id'],
            'ds_offset': ds0,
            'length': length,
            'manifest_kind': region['kind'],
            'classification': region.get('classification'),
            'skip': 0,
        }
        if spec is None:
            entry['kind'] = 'toolchain_opaque'
            entry['format'] = None
            gaps.append(entry)
        elif spec['format'] == 'compiled-data':
            entry['kind'] = 'code_owned'
            entry['format'] = 'compiled-data'
            entry['code_owner'] = spec['code_owner']
            entry['source_file'] = spec['source']
        else:
            data, refs, publics, extra = _structural_info(spec)
            skip = spec.get('skip', 0)
            if skip and refs:
                raise ValueError(f'{region["id"]}: cannot skip a pointer-bearing contribution')
            placed = data[skip:]
            if len(placed) + skip != length:
                raise ValueError(
                    f'{region["id"]}: structural encoding is {len(data)} bytes (skip={skip}), '
                    f'manifest region is {length} bytes')
            image[ds0 + skip:ds1] = placed
            # refs were computed against the *unsliced* encoder output; skip
            # only ever trims a leading span with no refs in it (asserted
            # above), so offsets need no adjustment.
            entry['kind'] = 'data'
            entry['format'] = spec['format']
            entry['refs'] = refs
            entry['publics'] = publics
            entry['extra'] = extra
            entry['source_file'] = spec['source']
            entry['skip'] = skip
        components.append(entry)

    if cursor != DATA_LEN:
        raise ValueError(f'DATA image covers {cursor:#x}, expected {DATA_LEN:#x}')

    return bytes(image), components, {'gaps': gaps}


def cross_check_against_exe(image, components):
    """OPTIONAL: if assets/AEPROG.EXE is present, assert every recipe-
    covered (non-pointer) DATA byte this generator produced matches the
    real historical EXE bit-for-bit. Returns a small report dict, or
    {'checked': False, ...} when the EXE is absent -- generation must (and
    does) succeed either way; this never contributes bytes to `image`.
    """
    if not EXE_PATH.exists():
        return {'checked': False, 'reason': 'assets/AEPROG.EXE not present (optional cross-check skipped)'}
    exe_bytes = EXE_PATH.read_bytes()
    manifest = read_json(MANIFEST_PATH)
    if len(exe_bytes) != manifest['original']['size']:
        return {'checked': False, 'reason': 'assets/AEPROG.EXE size does not match layout/manifest.json'}
    if hashlib.sha256(exe_bytes).hexdigest() != manifest['original']['sha256']:
        return {'checked': False, 'reason': 'assets/AEPROG.EXE sha256 does not match layout/manifest.json'}
    base = manifest['frames']['DGROUP'] + 512
    exe_data_slice = exe_bytes[base:base + DATA_LEN]

    mismatches = []
    checked_bytes = 0
    for c in components:
        if c['kind'] != 'data':
            continue
        ds0, ds1, skip = c['ds_offset'], c['ds_offset'] + c['length'], c['skip']
        expected = exe_data_slice[ds0 + skip:ds1]
        actual = image[ds0 + skip:ds1]
        masked_expected, masked_actual = bytearray(expected), bytearray(actual)
        for ref in c.get('refs', []):
            width = 4 if ref.get('loc', 'pointer32') == 'pointer32' else 2
            for i in range(ref['offset'], ref['offset'] + width):
                masked_expected[i] = 0
                masked_actual[i] = 0
        checked_bytes += len(masked_expected)
        if bytes(masked_expected) != bytes(masked_actual):
            mismatches.append(c['id'])
    return {'checked': True, 'components_checked': sum(1 for c in components if c['kind'] == 'data'),
            'bytes_checked': checked_bytes, 'mismatches': mismatches}



# ---------------------------------------------------------------------------
# Step B -- merged DGROUP symbol table
# ---------------------------------------------------------------------------

G_NAME_RE = re.compile(r'^[gG]([0-9a-fA-F]{1,4})$')

# Source (vi): names matching /^[gbs]0?[0-9a-f]{2,4}$/ in src/*.C and
# include/*.H encode their own DS offset in hex -- g94 -> 0x0094,
# gb6a -> 0x0B6A, g0b3ae -> BSS 0xB3AE, b856 -> 0x0856. 'b'/'s' are the same
# convention observed for byte-sized scratch variables ('b', e.g. b856) and
# DATA strings ('s', e.g. STARTUP.C's `extern char s859[], s8a8[];`,
# BOARD.C's `s8c12[]`/`s79bf[]`/`s7400[]`/`s735e[]`); 3-4 hex digits cover
# the DGROUP range, an optional literal '0' lets a 2-digit offset
# (0x0859 -> "859") sit next to 4-digit ones without ambiguity.
NAME_CONVENTION_RE = re.compile(r'^[gbs]0?([0-9a-f]{2,4})$', re.IGNORECASE)
S_STRING_NAME_RE = re.compile(r'^s0?[0-9a-f]{2,4}$', re.IGNORECASE)


def _is_s_string_name(name):
    """An 's'-prefixed name-convention name implies dos_char[] (a string)
    when nothing stronger types the same offset -- see resolve_symbols."""
    return bool(name) and bool(S_STRING_NAME_RE.match(name))


def _offset_from_g_name(raw):
    """`g3924`/`g0bc`/`g00bc` all name DS-relative offset 0x3924/0xbc/0xbc --
    the systematic OMF-recovery name for an object with no historical public
    (see docs/portable/architecture.md and tools/portable/state_ownership.json,
    which lists members of exactly this family)."""
    if not raw:
        return None
    m = G_NAME_RE.match(raw)
    if not m:
        return None
    return int(m.group(1), 16)


def _offset_from_name_convention(name):
    """Source (vi). Returns None for anything not matching the pattern, or
    whose decoded offset falls outside the DGROUP range (defensive: avoids
    treating an unrelated short identifier that happens to be all-hex
    letters, e.g. a name ending in a run of a-f digits, as an address)."""
    if not name:
        return None
    m = NAME_CONVENTION_RE.match(name)
    if not m:
        return None
    offset = int(m.group(1), 16)
    return offset if 0 <= offset < DGROUP_LEN else None


PORTED_C_TOPLEVEL_DEF_RE = re.compile(
    r'^(dos_char|dos_uchar|dos_int|dos_uint|dos_long|dos_ulong)\s+(\**)\s*(\w+)\s*(\[[^\]]*\])?\s*(?:=\s*[^;]+)?;')


def scan_ported_c_definitions():
    """portable/game/*.c top-level (column-0 -- i.e. file-scope, not a
    function-local declaration, which is always indented in this
    codebase's style) global variable DEFINITIONS (point 5, "ported-C-
    owned DATA"): a code_owned recipe component's real storage is one of
    these (e.g. portable/game/prompts.c's `dos_int gb80 = 4;`, the real
    definition behind DATA_0107B0_TABLE/code_owner F_75F3) -- game_data.h
    needs an `extern` DECLARATION for it (never a second definition; the
    .c file already has the one and only definition) so any OTHER ported
    unit that references the name by its historical g-name can see it.

    Returns name -> (file_path_relative_to_ROOT, declaration_text, i.e.
    the type/stars/name/dims with no initializer and no trailing ';').
    """
    out = {}
    for path in sorted((ROOT / 'portable/game').glob('*.c')):
        text = path.read_text(encoding='utf-8')
        for line in text.splitlines():
            m = PORTED_C_TOPLEVEL_DEF_RE.match(line)
            if not m:
                continue
            base, stars, name, dims = m.groups()
            decl = f"{base} {stars}{name}{dims or ''}"
            out[name] = (str(path.relative_to(ROOT)).replace('\\', '/'), decl)
    return out


C_KEYWORDS_FOR_FIELD_SCAN = frozenset({
    'void', 'struct', 'union', 'enum', 'unsigned', 'signed', 'char', 'int', 'long', 'short',
    'const', 'far', 'near', 'huge',
})
STRUCT_OPEN_RE = re.compile(r'\bstruct(?:\s+[A-Za-z_]\w*)?\s*\{')
FIELD_TOKEN_RE = re.compile(r'\b([A-Za-z_]\w*)\s*(?:\[[^\]]*\])?\s*[;,)]')

_STRUCT_FIELD_NAMES_CACHE = None


def scan_struct_field_names():
    """Every field name declared inside any struct body (including a
    nested anonymous one, e.g. g0dcc_entry's `struct { idx; sel; } e[11];`)
    in portable/include/game_structs.h or game_funcs.h -- used to veto a
    `#define` alias whose name would collide with, and silently rewrite,
    a real struct field name (`.f2`, `.off`, ...) wherever a ported .c
    file writes `x.f2` after including these headers.
    """
    global _STRUCT_FIELD_NAMES_CACHE
    if _STRUCT_FIELD_NAMES_CACHE is None:
        names = set()
        for rel in ('portable/include/game_structs.h', 'portable/include/game_funcs.h'):
            path = ROOT / rel
            if not path.exists():
                continue
            text = path.read_text(encoding='utf-8')
            text = re.sub(r'/\*.*?\*/', ' ', text, flags=re.DOTALL)
            text = re.sub(r'//[^\n]*', ' ', text)
            pos = 0
            while True:
                m = STRUCT_OPEN_RE.search(text, pos)
                if not m:
                    break
                depth, j = 1, m.end()
                while j < len(text) and depth > 0:
                    if text[j] == '{':
                        depth += 1
                    elif text[j] == '}':
                        depth -= 1
                    j += 1
                body = text[m.end():j - 1]
                for fm in FIELD_TOKEN_RE.finditer(body):
                    tok = fm.group(1)
                    if tok not in C_KEYWORDS_FOR_FIELD_SCAN and not tok.startswith('dos_'):
                        names.add(tok)
                pos = j
        _STRUCT_FIELD_NAMES_CACHE = names
    return _STRUCT_FIELD_NAMES_CACHE


def alias_macro_block_reason(name, blocked_field_names):
    """None if `#define name ...` is safe to emit; otherwise why it must
    NOT be (supervisor decision): as an OBJECT-LIKE macro, a short,
    un-prefixed name like `f2`/`off`/`err` silently rewrites any unrelated
    identifier spelled the same way -- a struct field, a local variable --
    in EVERY ported .c file that includes the header, not just uses of
    this one alias. Length alone doesn't distinguish a safe historical
    name (most are already g-prefixed/hex, naturally >= 4 chars) from a
    dangerous short common word, so anything under 4 characters is always
    blocked; a name of any length that happens to equal a real struct
    field name in game_structs.h/game_funcs.h is blocked for the same
    reason (`x.f2` must never expand through a `#define f2 ...`)."""
    if len(name) < 4:
        return 'short (<4 chars)'
    if name in blocked_field_names:
        return 'collides with a struct field name (game_structs.h/game_funcs.h)'
    return None


def load_state_ownership():
    doc = read_json(STATE_OWNERSHIP_PATH)
    owned = set()
    for names in doc['owners'].values():
        owned.update(names)
    return owned


def load_emit_qualifiers():
    """tools/portable/state_ownership.json['emit_qualifiers']: primary name
    -> a C qualifier ('volatile', ...) to prepend to both the extern
    declaration and the definition datagen.py emits for it."""
    doc = read_json(STATE_OWNERSHIP_PATH)
    return dict(doc.get('emit_qualifiers', {}))


def _qualify_decl(decl, qualifier):
    return decl.replace('extern ', f'extern {qualifier} ', 1)


def _qualify_defn(defn, qualifier):
    return f'{qualifier} {defn}'


# tools/portable/datagen_overrides.json field kinds -> (byte width, is a
# pointer field that needs resolving through component refs).
OVERRIDE_FIELD_KIND = {
    'farptr': (4, True), 'nearptr': (2, True),
    'u8': (1, False), 'i8': (1, False), 'u16': (2, False), 'i16': (2, False),
    'u32': (4, False), 'i32': (4, False),
}

# A 'struct' override's field kind list only says a pointer field's WIDTH
# (farptr/nearptr), not its declared C pointee type -- that comes from the
# real struct definition (portable/include/game_funcs.h for `struct
# input`, since it -- unlike game_structs.h's curated set -- is not one of
# KNOWN_STRUCT_SIZES/STRUCT_FIELD_LAYOUTS_FOR_ALIASING). Resolved pointer
# values are cast to it, per the coordinator's own worked example
# (`.records = (dos_char **)gc5ce`).
OVERRIDE_STRUCT_FIELD_C_TYPES = {
    'input': {'title': 'dos_char *', 'records': 'dos_char **'},
    'menu_record': {'label': 'dos_char *', 'text': 'dos_char *', 'callbacks': 'void (**)(void)'},
    'menu_catalog': {'records': 'struct menu_record *'},
    'dialog': {'title': 'dos_char *', 'text': 'dos_char *'},
}

# Which header declares a 'struct' override's tag, when it is not one of
# game_structs.h's KNOWN_STRUCT_SIZES (game_data.c needs the #include).
OVERRIDE_STRUCT_TAG_HEADER = {'input': 'game_funcs.h'}


def _parse_override_c_type(text):
    """'dos_uchar[385]' -> ('dos_uchar', [385]); 'struct gc316_tile' ->
    ('struct gc316_tile', []) -- datagen_overrides.json's plain (non-
    'struct') `c_type` string, split into the (base, dims) shape the rest
    of the generator already carries on every symbol."""
    dims = [v for v in (_c_int_literal(d) for d in ARRAY_DIM_RE.findall(text)) if v is not None]
    base = ARRAY_DIM_RE.sub('', text).strip()
    return base, dims


def load_datagen_overrides():
    """tools/portable/datagen_overrides.json: manual entries that win over
    every automatic derivation rule for their own [offset, offset+length)
    span -- for the handful of real historical objects no amount of
    generic symbol-driven inference gets right on its own: a struct
    reinterpreting two OTHER named scalars' storage (`gc132_tile` over
    `puzzle_held_piece`+`gc133`), a struct assembled from bytes that
    straddle two unrelated recipe components with no historical name of
    its own binding them together (`g22f0`, DIALOG.C's `struct input`),
    or a byte array the historical code walks with a +1 offset that no
    'skip'-style component encodes (`actor_record_table`).

    Schema (a JSON list): [{"name", "offset" (hex string), "length",
    "c_type" (optional plain "base[dims]" string), "struct" (optional
    {"tag", "fields": [[name, kind], ...]}, kind one of farptr/nearptr/
    u8/i8/u16/i16/u32/i32/bytes:N), "aliases" (optional {alias: "C
    expression"})}]. A 'struct' entry emits a designated initializer (DATA
    only -- see resolve_symbols/_emit_override_struct): farptr/nearptr
    fields are resolved through whatever recipe component's refs fall
    inside the override's own span, cast to OVERRIDE_STRUCT_FIELD_C_TYPES'
    declared field type, or a literal NULL when the raw bytes are all
    zero and nothing refs that offset. A plain `c_type` entry (DATA or
    BSS) just declares that type at that offset/length, no initializer
    logic. Either kind's `aliases` become `#define` macros with exactly
    the given expression text (chainable: one alias's expression may
    itself be another alias's name).
    """
    if not DATAGEN_OVERRIDES_PATH.exists():
        return []
    raw = read_json(DATAGEN_OVERRIDES_PATH)
    out = []
    for e in raw:
        off_field = e['offset']
        offset = int(off_field, 16) if isinstance(off_field, str) else off_field
        out.append({
            'name': e['name'], 'offset': offset, 'length': e['length'],
            'c_type': e.get('c_type'), 'struct': e.get('struct'), 'layout': e.get('layout'),
            'aggregate': e.get('aggregate', False), 'views': e.get('views'),
            'aliases': e.get('aliases') or {},
        })
    return out


class SymbolTable:
    """Accumulates (name -> offset) and (offset -> {names}) from every
    source, recording a conflict whenever the same name is asserted at two
    different offsets (the offset ambiguity itself -- not the eventual C
    *type* text -- since type conflicts are detected later, per name, once
    every declared type is known)."""

    def __init__(self):
        self.names_by_offset = {}   # int -> set(str)
        self.offset_by_name = {}    # str -> int
        self.tags_by_name = {}      # str -> set(str)  (which sources named it)
        self.conflicts = []         # [{'name', 'offsets', 'tags'}]

    def add(self, name, offset, tag):
        if not name or offset is None or not (0 <= offset < DGROUP_LEN):
            return
        self.names_by_offset.setdefault(offset, set()).add(name)
        self.tags_by_name.setdefault(name, set()).add(tag)
        prev = self.offset_by_name.get(name)
        if prev is None:
            self.offset_by_name[name] = offset
        elif prev != offset:
            self.conflicts.append({'name': name, 'offsets': sorted({prev, offset}), 'tag': tag})

    def names_at(self, offset):
        return self.names_by_offset.get(offset, set())


def build_symbol_table(components, externs, interface_conflicts_offsets=()):
    """Merge the six symbol sources named in the task brief into one table
    keyed by raw DGROUP offset (0 = DS:0000, DGROUP_LEN = end of BSS).

    Source priority for the *offset* of a given name, highest first (a
    later source's offset for an already-placed name is logged as a
    conflict and discarded, never silently overridden -- see
    `SymbolTable.add`): (v) component-public, (i) production-plan
    binding, (ii) GAME_BSS public, (iii) symbol-names.json, (iv) extern
    `/* DS:xxxx */` comment, (vi) name-convention fallback. (v) runs first
    only because (i)'s `owner`+`addend` bindings resolve against it, not
    because it should usually win; component ids/publics are a byte-source
    and last-resort naming fallback (see resolve_symbols), so in practice a
    real historical name from (i)-(iv) is what ends up primary.
    """
    manifest = read_json(MANIFEST_PATH)
    plan = read_json(PLAN_PATH)
    bss_doc = read_json(BSS_JSON_PATH)
    symnames = read_json(SYMNAMES_PATH)

    base = manifest['frames']['DGROUP'] + 512
    comp_by_id = {c['id']: c for c in components}
    table = SymbolTable()

    # (v) component ids + their component-local sub-publics (sound-data
    # lookup tables, typed-data's own `owner` name, ...).  This must run
    # first so (i)'s `owner`+`addend` bindings can resolve against it.
    for c in components:
        if c['kind'] != 'data':
            continue
        for pub_name, local_off in c['publics'].items():
            table.add(pub_name, c['ds_offset'] + local_off, 'component-public')

    # (i) layout/production-plan.json module build.bindings, DGROUP_offset
    for m in plan['modules']:
        for raw_name, info in m.get('build', {}).get('bindings', {}).items():
            if info.get('coordinate') != 'DGROUP_offset':
                continue
            if 'offset' in info:
                off = info['offset']
            elif 'owner' in info:
                owner = comp_by_id.get(info['owner'])
                if owner is None:
                    continue
                off = owner['ds_offset'] + info.get('addend', 0)
            else:
                continue
            name = raw_name[1:] if raw_name.startswith('_') else raw_name
            table.add(name, off, 'production-plan-binding')

    # (ii) src/data/GAME_BSS.json publics, offsets relative to DS:3902
    for raw_name, off in bss_doc['publics'].items():
        name = raw_name[1:] if raw_name.startswith('_') else raw_name
        table.add(name, DATA_LEN + off, 'game-bss-public')

    # (iii) docs/current/symbol-names.json: friendly name <-> historical g-name(s)
    friendly_of_offset = {}
    for friendly, info in symnames['names'].items():
        raws = [info.get('original')] + list(info.get('aliases', []))
        offs = set()
        for raw in raws:
            off = _offset_from_g_name(raw)
            if off is None:
                continue
            offs.add(off)
            table.add(raw, off, 'symbol-names-raw')
        for off in offs:
            table.add(friendly, off, 'symbol-names-friendly')
            friendly_of_offset.setdefault(off, friendly)

    # (iv) extern `/* DS:xxxx */` comments (any declaration parse_all_externs
    # found an explicit offset for) -- and (vii) interface-conflicts.json
    # typed entries merged into this same dict by generate(), each tagged
    # with its own `_source` so it registers under the right name here
    # instead of being misattributed as an ordinary extern-comment.
    for name, decls in externs.items():
        for decl in decls:
            if decl['offset'] is not None:
                table.add(name, decl['offset'], decl.get('_source', 'extern-comment'))

    # (vii) docs/current/interface-conflicts.json: every usable entry names
    # its offset even when it has no parseable declared type (an untyped
    # name here is still a real historical name, just like a
    # component-public or GAME_BSS public with no type of its own).
    for name, off in interface_conflicts_offsets:
        table.add(name, off, 'interface-conflicts')

    # GAME_BSS is the one pointer target name used by the recipe sources that
    # is neither a component id nor a historical g-name: it means "BSS base".
    table.add('GAME_BSS', DATA_LEN, 'bss-base-alias')

    # (vi) name-convention fallback -- lowest priority, so it only supplies
    # an offset for names none of (i)-(iv) placed; run last so any
    # disagreement with an already-placed binding is *logged*, not applied
    # (SymbolTable.add keeps the first-added offset for a name).
    for name in externs:
        if name in table.offset_by_name:
            continue
        off = _offset_from_name_convention(name)
        if off is not None:
            table.add(name, off, 'name-convention')

    return table, friendly_of_offset


# ---------------------------------------------------------------------------
# Step B continued -- historical extern declarations ("/* DS:XXXX */")
# ---------------------------------------------------------------------------

EXTERN_STMT_RE = re.compile(r'extern\s+([^;{}]+?)\s*;\s*(?:/\*(.*?)\*/)?')
DS_HEX_RE = re.compile(r'DS:([0-9A-Fa-f]{1,4})\b')
FUNC_PTR_DECL_RE = re.compile(r'\(\s*\*\s*(\w+)\s*\)\s*\(')
FUNC_PTR_ARRAY_DECL_RE = re.compile(r'\(\s*\*\s*(\w+)\s*(\[[^\]]*\](?:\s*\[[^\]]*\])*)\s*\)\s*\(')
FUNC_PROTO_RE = re.compile(r'\w\s*\(')


def _split_top_level_commas(text):
    """Split on commas that are not inside [] or () (array dims hold no
    commas here; parens matter for function-argument lists, e.g.
    'read(int,void far *,unsigned)', which must not split on the argument
    commas)."""
    parts, depth, start = [], 0, 0
    for i, ch in enumerate(text):
        if ch in '[(':
            depth += 1
        elif ch in '])':
            depth -= 1
        elif ch == ',' and depth == 0:
            parts.append(text[start:i])
            start = i + 1
    parts.append(text[start:])
    return parts
DECL_RE = re.compile(r'^(?P<pre>(?:.*[\s*])?)(?P<name>[A-Za-z_]\w*)(?P<dims>(?:\s*\[[^\]]*\])*)\s*$')
ARRAY_DIM_RE = re.compile(r'\[\s*([^\]]*)\s*\]')


def _c_int_literal(text):
    """Parse one array-dimension token as a C integer literal: plain
    decimal ('24') or hex ('0xe2'/'0XE2', common in this codebase for
    record widths -- e.g. `char g6f2a[][0xe2]`). Returns None for anything
    else (empty -- an unspecified dimension -- or an expression), so
    callers can still tell "no dimension given" apart from "dimension 0"."""
    text = text.strip()
    if not text:
        return None
    if text[:2].lower() == '0x':
        try:
            return int(text, 16)
        except ValueError:
            return None
    return int(text) if text.isdigit() else None

PRIMITIVE_C_TYPE = {
    'char': 'dos_char', 'signed char': 'dos_char', 'unsigned char': 'dos_uchar',
    'int': 'dos_int', 'signed': 'dos_int', 'signed int': 'dos_int', 'short': 'dos_int',
    'unsigned': 'dos_uint', 'unsigned int': 'dos_uint', 'unsigned short': 'dos_uint',
    'long': 'dos_long', 'signed long': 'dos_long',
    'unsigned long': 'dos_ulong',
    'void': 'void',
}


def _split_declarator(decl):
    """'struct c470_record slot_table[10]' -> ('slot_table', 'struct c470_record', ['10'], False, False, False)
    'char near *dialog_button1_label_off' -> ('dialog_button1_label_off', 'char', [], True, False, True)
    'int (*foo)()' (a function-pointer *variable*, not a prototype) ->
        ('foo', 'void (*)(void)', [], False, True, False)
    Returns None for a plain function prototype ('open()', 'read(int,...)',
    'void far * memmove()') -- those name no DATA/BSS object at all.

    The 6th element, `is_near`, records whether `near` (not `far`) qualified
    a pointer -- the historical *span* of a pointer object is 2 bytes near,
    4 bytes far (compact model: code near, data far by default when
    unqualified), which matters for sizing an object from its offset to the
    next known symbol even though the portable object is always a native
    (8-byte) pointer."""
    decl = decl.strip()
    fpa = FUNC_PTR_ARRAY_DECL_RE.search(decl)
    if fpa:
        # An ARRAY of function pointers (`void (*g12a1[])(void)`), not a
        # single function-pointer variable -- same 'void (*)(void)' element
        # type, but with real array dims so it goes through the normal
        # array-sizing/reshape machinery (point: "code-pointer tables").
        dims = [d.strip() for d in ARRAY_DIM_RE.findall(fpa.group(2))]
        return fpa.group(1), 'void (*)(void)', dims, False, True, False
    fp = FUNC_PTR_DECL_RE.search(decl)
    if fp:
        return fp.group(1), 'void (*)(void)', [], False, True, False
    # A bare identifier immediately followed by '(' with no leading '*' is a
    # function prototype (declares a function, not a storage object).
    if FUNC_PROTO_RE.search(decl):
        return None
    m = DECL_RE.match(decl)
    if not m:
        return None
    name = m.group('name')
    dims = [d.strip() for d in ARRAY_DIM_RE.findall(m.group('dims'))]
    pre = m.group('pre').strip()
    is_ptr = pre.endswith('*')
    base = pre[:-1].strip() if is_ptr else pre
    is_near = bool(re.search(r'\bnear\b', base))
    base = re.sub(r'\b(near|far|huge|const)\b', '', base)
    base = re.sub(r'\s+', ' ', base).strip()
    return name, base, dims, is_ptr, False, is_near


# portable/include/game_structs.h -- hand-ported historical struct sizes
# (the HISTORICAL byte size, used for object-span computation; portable
# sizeof() can differ, e.g. struct dialog's two far pointers are now native
# 8-byte pointers, which is fine because objects are placed by DS-offset
# span, never by sizeof()).
KNOWN_STRUCT_SIZES = {
    'struct c470_record': 27, 'struct dialog': 20, 'struct g0dcc_entry': 24,
    'struct g2fd2_entry': 2, 'struct ga5e_entry': 0x23, 'struct gb3af_entry': 32,
    'struct gc0fe_record': 20, 'struct gc0fe_catalog': 6, 'struct gc316_tile': 2,
    'struct gc91b_entry': 14, 'struct record3e8': 0x3e8, 'struct tbl_entry': 27,
}

# Structs (per portable/include/game_structs.h) that are exactly one byte
# array field wrapped in a struct -- name of that field, for a safe
# `{ .field = {bytes} }` initializer with no field-layout ambiguity.
SINGLE_ARRAY_FIELD_STRUCTS = {
    'struct ga5e_entry': 'b', 'struct record3e8': 'bytes', 'struct gc91b_entry': 'f',
}

# Every OTHER pointer-free (portable/include/game_structs.h `#pragma
# pack(push, 1)`) struct's field widths, in declaration order (an array
# field repeats its element width `count` times; a nested anonymous
# struct's fields are flattened the same way) -- ISO C's brace-elision
# rule lets a flat, non-designated `{ v0, v1, ... }` initializer fill a
# nested aggregate correctly as long as the VALUES appear in this same
# order, so no field *names* are needed here, just byte widths, to emit a
# byte-exact `struct X name[N] = { {...}, {...}, ... };` instead of an
# untyped uint8_t fallback (point 3: "every historical struct type must
# emit its real struct, not uint8_t[]"). Sum of widths must equal
# KNOWN_STRUCT_SIZES[struct] (asserted in emit_generic_flat_object).
STRUCT_INIT_FIELD_WIDTHS = {
    # g0dcc_entry: struct { uchar idx; char sel; } e[11]; char f16; char f17;
    'struct g0dcc_entry': [1] * 22 + [1, 1],
    'struct g2fd2_entry': [1, 1],                    # b0; b1;
    'struct gb3af_entry': [1] * 32,                  # flag; rest[31];
    'struct gc316_tile': [1, 1],                     # kind; rot;
    # tbl_entry: pad0[9]; a9(int); b11; pad1; d13(int); f15(int); h17(int); j19(int); l21; pad2[5];
    'struct tbl_entry': [1] * 9 + [2, 1, 1, 2, 2, 2, 2, 1] + [1] * 5,
}

# Rule B: (offset, field name, element size, element count) for structs
# whose interior-alias field path we can name precisely; anything not
# listed here still gets a rule-B alias, just the generic
# `(*((dos_char *)base + N))` byte-offset form instead of `.field[i]`.
STRUCT_FIELD_LAYOUTS_FOR_ALIASING = {
    'struct c470_record': [
        (0, 'text', 1, 9), (9, 'value', 2, 1), (11, 'flags', 1, 1),
        (12, 'resume_round', 1, 1), (13, 'sound', 2, 1), (15, 'music', 2, 1),
        (17, 'option', 2, 1), (19, 'pending', 2, 1), (21, 'state', 1, 1),
        (22, 'round_progress', 1, 4), (26, 'byte26', 1, 1),
    ],
    # struct dialog (DIALOG_FIELD_LAYOUT below, restated as (off, name,
    # elem_size, count) for _struct_field_path) -- used by source (vii)
    # (docs/current/interface-conflicts.json) interior-alias names that
    # land inside a `ptrrec-dialog` record instead of at its own base
    # offset, e.g. g13b8 (DS:13B8) = dialog_slot_delete_confirm.text.
    'struct dialog': [
        (0, 'kind', 2, 1), (2, 'title', 4, 1), (6, 'sub', 1, 1), (7, 'text', 4, 1),
        (11, 'initial', 1, 1), (12, 'cx', 2, 1), (14, 'cy', 2, 1), (16, 'w', 2, 1),
        (18, 'lines', 2, 1),
    ],
}


def _struct_field_path(struct_name, sub_offset):
    """'.text[0]' etc. for a byte offset inside a known struct, or None if
    unknown/misaligned (rule B then falls back to a raw byte-offset cast)."""
    for off, name, elem_size, count in STRUCT_FIELD_LAYOUTS_FOR_ALIASING.get(struct_name, ()):
        if off <= sub_offset < off + elem_size * count:
            rel = sub_offset - off
            if rel % elem_size:
                return None
            idx = rel // elem_size
            return f'.{name}[{idx}]' if count > 1 else f'.{name}'
    return None


# Rule F: primary name -> synthetic union object name, for a declared type
# that genuinely overlaps a neighboring struct-shaped component's storage
# (confirmed historical aliasing, not a data-quality error to clip around).
STORAGE_ALIAS_UNIONS = {
    'ui_panel_glyph_records': 'sound_instrument_region',
}


def resolve_primitive(base):
    """Map a historical base-type string to a dos_* alias or a known
    portable/include/game_structs.h struct name, or (None, note)."""
    if base == 'void (*)(void)':
        return base, None  # already a complete function-pointer type
    if base in KNOWN_STRUCT_SIZES:
        return base, None  # portable/include/game_structs.h defines this exact type
    if base in ('far pointer offset word', 'far pointer segment word'):
        return 'dos_uint', 'stored as one half of a historical far pointer pair'
    if base == 'jmp_buf':
        # Rule G: the historical 20 bytes were Turbo C's jmp_buf layout;
        # the port uses the host <setjmp.h> jmp_buf instead (game_state.h
        # includes it whenever this type is used -- see emit_game_state).
        return 'jmp_buf', None
    if base.startswith('struct ') or base.startswith('union ') or base.startswith('enum '):
        return None, f'historical {base!r} has no portable definition yet'
    mapped = PRIMITIVE_C_TYPE.get(base)
    if mapped is None:
        return None, f'unrecognized historical type {base!r}'
    return mapped, None


def historical_elem_size(base, is_ptr, is_near, mapped):
    """Byte width of ONE element for span computation -- dos_* primitive
    width, a known struct's historical size, or a pointer's historical
    (near=2/far=4) width. A scalar function pointer (rule A/H: an
    interrupt vector etc.) keeps the default FAR (4-byte) width -- the
    real IVT stores a segment:offset pair -- but see the had_array-only
    override in resolve_symbols' reshape branch for an ARRAY of function
    pointers (a compact-model code-pointer TABLE, e.g. ROUNDEND.C's
    g12a1[]), which really is 2 bytes/entry (near: this program's own
    single code segment, not a far interrupt vector)."""
    if is_ptr or base == 'void (*)(void)':
        return _historical_ptr_width(is_near)
    if base in KNOWN_STRUCT_SIZES:
        return KNOWN_STRUCT_SIZES[base]
    return ELEM_SIZE_OF.get(mapped, 1)


def parse_all_externs():
    """Scan every `extern <decl>;` statement in src/*.C and include/*.H --
    the full historical TYPE INVENTORY, independent of whether a `/* DS:xxxx
    */` comment is present (most extern re-declarations across TUs have
    none; the DS: comment is only how *this* scan learns an offset without
    consulting any other source -- see resolve_offset_sources()).

    Handles multi-declarator lines (`extern int g857, g94, g96, ...;`,
    `extern int near key_up_held,gb6a,...;`), function *prototypes*
    (skipped: they name no storage object), and function-pointer
    *variables* (`int (*foo)()`  -> type 'void (*)(void)').

    Returns name -> list of {offset_or_None, type_text, base, dims, is_ptr,
    file, line} (a list because the same name is very often re-declared,
    identically or not, in many TUs; genuine type disagreement becomes the
    'Alias type conflicts' state-map.md section).
    """
    results = {}
    for pattern in ('include/*.H', 'src/*.C'):
        for path in sorted(ROOT.glob(pattern)):
            text = path.read_text(encoding='latin-1')
            for lineno, line in enumerate(text.splitlines(), 1):
                m = EXTERN_STMT_RE.search(line)
                if not m:
                    continue
                decl_text, comment = m.group(1).strip(), m.group(2)
                hexoffs = DS_HEX_RE.findall(comment) if comment else []
                declarators = _split_top_level_commas(decl_text)
                # A DS: comment must pair 1:1 with the declarator list (a
                # single declarator + single DS: address is the overwhelming
                # common case); anything else either has no comment (offsets
                # come from elsewhere) or is ambiguous and is treated as
                # having no comment rather than guessed -- EXCEPT the one
                # other unambiguous shape: a single (possibly pointer-
                # array) declarator with an "X offset, Y segment" comment
                # (e.g. `extern char far *gbfee[]; /* DS:BFEE offset,
                # DS:BFF0 segment */`) -- same two-hex pattern rule A
                # already resolves for a lone far pointer, just also
                # covering the array-of-far-pointers spelling; the first
                # hex is this declarator's own offset (the second, the
                # +2 segment half, is absorbed by rule A elsewhere, never
                # its own span).
                if len(hexoffs) == len(declarators):
                    offsets = [int(h, 16) for h in hexoffs]
                elif len(declarators) == 1 and len(hexoffs) == 2:
                    offsets = [int(hexoffs[0], 16)]
                else:
                    offsets = [None] * len(declarators)
                shared_base = None
                for declarator, offset in zip(declarators, offsets):
                    split = _split_declarator(declarator)
                    if split is None:
                        continue  # function prototype: no storage object
                    name, base, dims, is_ptr, is_func_ptr, is_near = split
                    if not is_func_ptr:
                        if shared_base is None:
                            shared_base = base
                        elif not base:
                            base = shared_base
                    entry = {
                        'offset': offset, 'type_text': decl_text, 'base': base,
                        'dims': dims, 'is_ptr': is_ptr, 'is_func_ptr': is_func_ptr,
                        'is_near': is_near,
                        'file': str(path.relative_to(ROOT)).replace('\\', '/'), 'line': lineno,
                    }
                    results.setdefault(name, []).append(entry)
    return results


INTERFACE_CONFLICTS_PATH = ROOT / 'docs' / 'current' / 'interface-conflicts.json'
C_IDENT_RE = re.compile(r'^[A-Za-z_]\w*$')


def load_interface_conflicts():
    """Source (vii): docs/current/interface-conflicts.json's `globals`
    array -- an independent OMF/interface-census cross-check that names a
    handful of DGROUP offsets none of sources (i)-(vi) do, or corrects an
    offset-fallback name to the real historical one (e.g. a `ptrrec-dialog`
    record generic-named `dialog_13C5` because nothing else named DS:13C5
    is really `menu_empty_record`, per SLOTMENU.C).

    Each entry's `storage_evidence[].offset` (falling back to
    `known_storage_ranges[0][0]` when there is no storage_evidence) gives
    the DGROUP offset; `declarations[0].type`/`.array` (when present) gives
    the historical type, parsed through the same `_split_declarator`
    machinery as any other extern so it participates in the normal
    widest/most-structured-wins type contest and 'Alias type conflicts'
    reporting -- just tagged 'interface-conflicts' (source vii, ranked
    below the more directly-sourced (i)-(v) in NAME_SOURCE_PRIORITY, since
    this source's own `confidence` levels run LOW/MEDIUM/UNKNOWN) instead
    of 'extern-comment' (source iv).

    Returns (offsets, typed_entries): `offsets` is [(name, offset), ...]
    for every usable entry (even an untyped one still names its offset, the
    same way a component-public or GAME_BSS public can); `typed_entries` is
    name -> [entry, ...] in the exact shape parse_all_externs() produces
    (plus `_source`), ready to merge into that dict.
    """
    if not INTERFACE_CONFLICTS_PATH.exists():
        return [], {}
    doc = read_json(INTERFACE_CONFLICTS_PATH)
    offsets = []
    typed_entries = {}
    for entry in doc.get('globals', ()):
        name = entry.get('symbol')
        if not name or not C_IDENT_RE.match(name):
            continue  # e.g. 'LDIV@'/'8087': not a valid C identifier, skip
        offset = None
        for se in entry.get('storage_evidence', ()):
            off = se.get('offset')
            if isinstance(off, int):
                offset = off
                break
        if offset is None:
            for lo, *_rest in entry.get('known_storage_ranges', ()):
                offset = lo
                break
        if offset is None or not (0 <= offset < DGROUP_LEN):
            continue
        offsets.append((name, offset))
        decls = entry.get('declarations') or []
        if not decls or not decls[0].get('type'):
            continue
        d = decls[0]
        decl_text = f"{d['type']} {name}{d.get('array') or ''}"
        split = _split_declarator(decl_text)
        if split is None:
            continue  # a function prototype-shaped 'type' text: skip
        _, base, dims, is_ptr, is_func_ptr, is_near = split
        typed_entries.setdefault(name, []).append({
            'offset': offset, 'type_text': decl_text, 'base': base, 'dims': dims,
            'is_ptr': is_ptr, 'is_func_ptr': is_func_ptr, 'is_near': is_near,
            'file': d.get('file') or 'docs/current/interface-conflicts.json',
            'line': d.get('line') or 0, '_source': 'interface-conflicts',
        })
    return offsets, typed_entries


# ---------------------------------------------------------------------------
# Step B continued -- BSS typed_reserves (layout/production-plan.json['bss'])
# ---------------------------------------------------------------------------

def load_bss_typed_reserves():
    """Absolute-DGROUP-offset -> {name, dims, base, count, element_bytes,
    source_type} for every layout/production-plan.json['bss'][*]['typed_reserves']
    entry.  `dims` is parsed from the literal source_type text (e.g.
    'unsigned char[226][4]' -> [226, 4]); count*element_bytes is asserted to
    equal the product of dims * sizeof(base) as a cross-check, since the two
    fields encode the same span two different (and, for >1-D arrays,
    differently shaped) ways.
    """
    plan = read_json(PLAN_PATH)
    out = {}
    for block in plan['bss']:
        for tr in block.get('typed_reserves', ()):
            abs_off = DATA_LEN + block['logical_start'] + tr['offset']
            raw_base = ARRAY_DIM_RE.sub('', tr['source_type']).strip()
            dims_text = ARRAY_DIM_RE.findall(tr['source_type'])
            dims = [v for v in (_c_int_literal(d) for d in dims_text) if v is not None]
            is_ptr = raw_base.endswith('*')
            if raw_base in ('far pointer offset word', 'far pointer segment word'):
                base = raw_base  # checked verbatim in resolve_primitive; do not strip 'far'
            else:
                base = raw_base[:-1].strip() if is_ptr else raw_base
                base = re.sub(r'\b(near|far|huge)\b', '', base)
                base = re.sub(r'\s+', ' ', base).strip()
            mapped, note = resolve_primitive(base)
            elem_size = {'dos_char': 1, 'dos_uchar': 1, 'dos_int': 2, 'dos_uint': 2,
                         'dos_long': 4, 'dos_ulong': 4}.get(mapped, tr['element_bytes'])
            product = 1
            for d in dims:
                product *= d
            expected_total = tr['count'] * tr['element_bytes']
            if dims and product * elem_size != expected_total and mapped is not None:
                # Not fatal (a handful of historical annotations are informal);
                # fall back to the flat count*element_bytes span for byte
                # correctness and drop the multi-dim shape.
                dims = []
            out[abs_off] = {
                'name': tr['name'], 'base': base, 'dims': dims, 'is_ptr': is_ptr,
                'count': tr['count'], 'element_bytes': tr['element_bytes'],
                'source_type': tr['source_type'], 'span': expected_total,
                'block': block['id'],
            }
    return out


# ---------------------------------------------------------------------------
# Step B continued -- resolve one entry per claimed DGROUP span
# ---------------------------------------------------------------------------

NAME_SOURCE_PRIORITY = (
    'symbol-names-friendly', 'extern-comment', 'game-bss-public',
    'production-plan-binding', 'symbol-names-raw', 'interface-conflicts',
    'name-convention', 'component-public',
)

# typed-data-v1/empires-sound-data-v1/empires-sound-instruments-v1/
# u16-farptr-u8-farptr-u8-tail8-v1 components carry pointer32/offset16 refs
# that only this generator's own struct-field machinery can resolve, so
# they always remain ONE claimed span at their component base (never split
# by an internally-named field) -- see docs/portable/state-map.md's
# "struct dialog" note for how a real historical struct type still gets
# applied to one of these when it matches.
STRUCT_SHAPED_FORMATS = (TYPED_FORMAT, SOUND_FORMAT, PTRREC_FORMAT, INSTR_FORMAT)

ELEM_SIZE_OF = {'dos_char': 1, 'dos_uchar': 1, 'dos_int': 2, 'dos_uint': 2,
                'dos_long': 4, 'dos_ulong': 4}


def _pick_primary(names, table, friendly_of_offset, offset):
    """Historical C symbol always wins over a component id (source (v),
    lowest priority): a real name from any of sources (i)-(iv)/(vi) is
    preferred; the component id is used only when nothing else names the
    object at all."""
    if offset in friendly_of_offset and friendly_of_offset[offset] in names:
        return friendly_of_offset[offset]
    ranked = []
    for name in names:
        tags = table.tags_by_name.get(name, set())
        rank = min((NAME_SOURCE_PRIORITY.index(t) for t in tags if t in NAME_SOURCE_PRIORITY),
                   default=len(NAME_SOURCE_PRIORITY))
        ranked.append((rank, name))
    ranked.sort(key=lambda t: (t[0], t[1]))
    return ranked[0][1]


def _historical_ptr_width(is_near):
    return 2 if is_near else 4  # compact model: unqualified/`far` data pointer is 4 bytes


def _type_rank(entry):
    """Widest/most-structured-wins ranking for point 3's "several names at
    one offset disagree in type" rule."""
    base = entry['base']
    is_struct = base.startswith('struct ') or base.startswith('union ')
    is_funcptr = base == 'void (*)(void)'
    mapped, _ = resolve_primitive(base)
    elem_size = historical_elem_size(base, entry['is_ptr'], entry.get('is_near', False), mapped)
    dims_product, has_unspecified_dim = 1, False
    for d in entry['dims']:
        v = _c_int_literal(d)
        if v is not None:
            dims_product *= v
        else:
            has_unspecified_dim = True
    span = elem_size * dims_product
    return (1 if is_struct else 0, 1 if (entry['is_ptr'] or is_funcptr) else 0, span,
            1 if has_unspecified_dim else 0)


def _types_compatible(a, b):
    """Whether an alias name's type is close enough to the chosen primary's
    to safely `#define alias primary` (same resolved storage kind); if not,
    the alias needs a supervisor-reviewed comment instead (state-map.md's
    "Alias type conflicts")."""
    a_mapped, _ = resolve_primitive(a['base'])
    b_mapped, _ = resolve_primitive(b['base'])
    if a['base'] == b['base'] and a['is_ptr'] == b['is_ptr']:
        return True
    return a_mapped is not None and a_mapped == b_mapped and a['is_ptr'] == b['is_ptr']


# Sources whose evidence, ON ITS OWN, is too weak to anchor a span
# boundary or win a type contest: a bare offset-derived identifier
# (name-convention, or interface-conflicts' own g-prefixed placeholder for
# an entry with no declared type at all -- e.g. `g0b78`, `type_confidence:
# "UNKNOWN"`, zero `declarations`) or a component's own generated id.
WEAK_ONLY_TAGS = frozenset({'name-convention', 'component-public', 'interface-conflicts'})


def _is_weak_name(name, table, externs):
    """A name is 'weak' when every source that ever named it is drawn from
    WEAK_ONLY_TAGS *and* it has no real declared type anywhere in `externs`
    (a typed interface-conflicts entry, e.g. `g13b8`/`menu_empty_record`,
    carries genuine src/*.C-sourced type evidence and is NOT weak just
    because its only *offset* tag happens to be 'interface-conflicts').
    Such a name must never terminate/clip another, strongly-evidenced
    span, and -- per supervisor decision -- if it lands inside one anyway
    it becomes an element/field alias of that object instead of its own
    boundary."""
    tags = table.tags_by_name.get(name, set())
    if not tags or not tags <= WEAK_ONLY_TAGS:
        return False
    return not externs.get(name)


def resolve_historical_type(names, externs, table, primary=None):
    """Gather every extern declaration for any name in `names`; pick the
    widest/most-structured as the historical type (point 3), and report the
    rest as alias-type information (compatible -> #define is fine;
    otherwise -> needs a supervisor-reviewed comment).

    A name whose *only* offset source is (vi) name-convention (no DS:
    comment, no binding, nothing corroborating it) is excluded from
    *winning* the widest-type contest, though it is still reported as an
    alias: the convention can occasionally collide with an unrelated real
    name (e.g. `ba22` happens to name-convention-decode to the same offset
    as the real, DS:-commented `ga22`), and an uncorroborated guess must
    never override or clip a confirmed placement.

    Returns (chosen_entry_or_None, chosen_name_or_None, alias_reports) where
    alias_reports is [{'name', 'entry', 'compatible'}] for every
    *distinctly-shaped* declaration that was not chosen.
    """
    # `names` is a set; iterate it in a fixed (sorted) order so `candidates`
    # -- and therefore `alias_reports` below, which several output paths
    # (state-map.md's "Alias type conflicts" table, symbols.json's
    # `alias_type_reports`) emit in list order -- doesn't depend on
    # Python's per-process hash-randomized set iteration order.
    candidates = [(n, d) for n in sorted(names) for d in externs.get(n, ())]
    if not candidates:
        return None, None, []
    corroborated = [(n, d) for n, d in candidates
                     if table.tags_by_name.get(n, set()) != {'name-convention'}]
    pool = corroborated or candidates
    # Deterministic tiebreak for an equal-rank tie between two names at the
    # same offset (e.g. MUSIC.C's `unsigned gca6d[]` vs. OPLREG.C's
    # `int tab_bias[]`, identical width/shape): prefer the resolved
    # PRIMARY name's own declaration, then sort by name, so the choice
    # never depends on set/dict iteration order.
    chosen_name, chosen = max(pool, key=lambda nd: (_type_rank(nd[1]), nd[0] == primary, nd[0]))
    seen_shapes = {(chosen['base'], tuple(chosen['dims']), chosen['is_ptr'])}
    alias_reports = []
    for n, d in candidates:
        shape = (d['base'], tuple(d['dims']), d['is_ptr'])
        if shape in seen_shapes:
            continue
        seen_shapes.add(shape)
        alias_reports.append({'name': n, 'entry': d, 'compatible': _types_compatible(chosen, d)})
    return chosen, chosen_name, alias_reports


def _historical_span(entry):
    """Byte span implied by a resolved extern type: None when unbounded
    (an unspecified outer array dimension, so the next-known-symbol rule
    must size it instead)."""
    if entry is None:
        return None
    base = entry['base']
    mapped, _ = resolve_primitive(base)
    if mapped is None and base not in KNOWN_STRUCT_SIZES:
        return None  # unresolved struct/jmp_buf/etc: fall back to next-symbol sizing
    elem_size = historical_elem_size(base, entry['is_ptr'], entry.get('is_near', False), mapped)
    product = 1
    for d in entry['dims']:
        v = _c_int_literal(d)
        if v is None:
            return None  # an unspecified dimension: bounded by next symbol instead
        product *= v
    return elem_size * product


def _split_sound_component(component, table, friendly_of_offset, externs):
    """Supervisor round 3: DATA_01139E_SOUND (empires-sound-data-v1) must be
    symbol-split like a flat component, not left as one opaque struct --
    include/SOUND.H already gives almost every word in [DS:176E, DS:1E96) a
    real extern with a DS: comment, so the ordinary per-offset historical-
    type resolution just works once let loose inside this component's own
    byte range. Three kinds of local span result:

      * named  -- a real historical symbol (SOUND.H extern, or this
                  component's own 'note_divisors'/'note_divisors_octave'/
                  'lookup_XXXX' publics): typed/sized exactly as the main
                  walk would (reusing resolve_historical_type/
                  _historical_span), falling back to next-boundary sizing
                  for the two publics (they have no extern type).
      * ptr_array -- a run of consecutive, UNNAMED pointer32/offset16 refs
                  of the same width (the dispatch table at DS:1832, the two
                  note-bank pointers at DS:182C): one `void *name[N]`
                  array, named by its own DS offset since nothing else
                  names it (SOUND.ASM addresses it by raw displacement).
      * byte_region -- bytes with no name and no ref (the LOW-confidence
                  17C4/17CC/17D4/17DC/17E4/17F4 words docs/current/
                  sound-state.md documents as ASM-internal, and the tail
                  words after the last lookup table): `uint8_t
                  sound_region_XXXX[N]`, named by DS offset, so the whole
                  1832-byte span still round-trips exactly.

    Returns (symbols, warnings); a warning fires if a pointer ref is ever
    found straddling a named symbol's boundary (asked for explicitly --
    none are expected, since every SOUND.H field is word-granular and every
    ref is a whole word).
    """
    comp_base, length, comp_id = component['ds_offset'], component['length'], component['id']
    refs_by_off = {r['offset']: r for r in component['refs']}
    ref_width = {off: (4 if r.get('loc', 'pointer32') == 'pointer32' else 2)
                 for off, r in refs_by_off.items()}

    def names_at(local):
        return table.names_at(comp_base + local) - {comp_id}

    symbols, warnings = [], []
    local = 0
    while local < length:
        names = names_at(local)
        if not names and local not in refs_by_off:
            start = local
            local += 1
            while local < length and not names_at(local) and local not in refs_by_off:
                local += 1
            size = local - start
            abs_off = comp_base + start
            ident = f'sound_region_{abs_off:04X}'
            symbols.append(_make_symbol(abs_off, size, 'data', ident, [], [ident], None,
                                         'generated', comp_id, None, None, [], False, [], True))
            continue

        if local in refs_by_off and not names:
            start, width = local, ref_width[local]
            run = []
            while (local < length and local in refs_by_off and ref_width.get(local) == width
                   and not (names_at(local) if local != start else set())):
                run.append(refs_by_off[local])
                local += width
            size = local - start
            abs_off = comp_base + start
            ident = f'sound_dispatch_{abs_off:04X}'
            symbols.append({
                'offset': abs_off, 'size': size, 'section': 'data', 'primary': ident,
                'aliases': [], 'names': [ident], 'owned_by': None, 'definition_site': 'generated',
                'component_id': comp_id, 'c_type': None, 'type_note': None, 'dims': [len(run)],
                'is_ptr': True, 'untyped': False, 'conflict': False, 'emit_path': 'sound-ptr-array',
                'alias_type_reports': [], 'refs': run, 'ref_width': width,
            })
            continue

        # Named (SOUND.H extern, or a component-local public like
        # note_divisors/lookup_0119). Straddle check first: a ref must
        # never start strictly inside a named object (only ever cleanly
        # at/after its end), and a named object boundary must never fall
        # strictly inside a ref's own width.
        abs_off = comp_base + local
        primary = _pick_primary(names, table, friendly_of_offset, abs_off)
        aliases = sorted(n for n in names if n != primary)
        chosen, chosen_name, alias_reports = resolve_historical_type(names, externs, table, primary)
        c_type = dims = is_ptr = type_note = None
        untyped = True
        size = None
        if chosen is not None:
            mapped, note = resolve_primitive(chosen['base'])
            is_ptr = chosen['is_ptr'] or chosen['base'] == 'void (*)(void)'
            if chosen['base'] == 'void (*)(void)':
                mapped = 'void'
            c_type, type_note = mapped, note
            dims = [v for v in (_c_int_literal(d) for d in chosen['dims']) if v is not None]
            untyped = mapped is None
            size = _historical_span(chosen)
        if size is None:
            nxt = local + 1
            while nxt < length and not names_at(nxt) and nxt not in refs_by_off:
                nxt += 1
            size = nxt - local
            c_type, dims, is_ptr, untyped = None, [], False, True
        for k in range(local + 1, local + size):
            if k in refs_by_off:
                warnings.append(f'DATA_01139E_SOUND: pointer ref at local {k:#x} straddles named '
                                 f'object {primary!r} ({local:#x}..{local + size:#x}) -- needs a '
                                 'supervisor decision, not auto-split.')
        symbols.append(_make_symbol(abs_off, size, 'data', primary, aliases, sorted(names),
                                     None, 'generated', comp_id, c_type, type_note, dims, is_ptr,
                                     alias_reports, untyped))
        local += size

    return symbols, warnings


def resolve_symbols(components, table, friendly_of_offset, externs, ownership, overrides=()):
    """Symbol-driven object model (recipe components are only a byte source
    and a last-resort naming fallback, per the supervisor review): walk
    DGROUP from DS:0000 to the end of BSS. At each position:

      * inside a structural DATA component (typed-data/sound-data/sound-
        instruments/pointer-records) or a code_owned/toolchain_opaque
        region: claim the whole region as one unit (no bytes for the
        latter two).
      * at a DGROUP offset some symbol names: claim an object there. Size
        priority is (1) a *fully* dimensioned historical array type (every
        `[N]` numeric) or scalar type -- authoritative regardless of what
        follows; (2) a *partially* dimensioned type (one empty `[]`,
        e.g. `char ga22[][16]`) or no resolved type at all -- sized out to
        the next protected-region start or the next named offset,
        whichever comes first (an explicit inner dimension then derives
        the outer count). A far/near pointer's *historical* span (4/2
        bytes) is used for this sizing even though the portable object is
        a native pointer.
      * otherwise (no symbol at this position): find the enclosing flat
        DATA component. If NO offset inside that component is ever named,
        emit one fallback object named after the component id, spanning
        from here to the component's end (recipe-format-aware: ascii
        string/u16 table/etc; see describe_and_emit_component). If some
        other offset in the component *is* named (this position is an
        un-named remainder -- e.g. DATA_00FCC2's un-skipped leading word
        at DS:0092, or a BSS gap), emit nothing and advance to the next
        boundary: "the word at DS:0092 has no symbol: emit nothing for it."
    """
    comp_owner_at = {}  # offset -> component (any kind) covering it
    for c in components:
        for off in range(c['ds_offset'], c['ds_offset'] + c['length']):
            comp_owner_at[off] = c
    component_has_any_name = {}
    for c in components:
        if c['kind'] != 'data' or c['format'] in STRUCT_SHAPED_FORMATS:
            continue
        component_has_any_name[c['id']] = any(
            table.names_at(o) for o in range(c['ds_offset'] + c['skip'], c['ds_offset'] + c['length']))

    typed_reserves = load_bss_typed_reserves()
    all_offsets = sorted(table.names_by_offset)
    # Offsets named by at least one STRONG name -- used as the boundary for
    # sizing/reshaping a wider historical array, so a weak name can never
    # truncate one (ga22[3][16] mid-shape, or -- supervisor decision --
    # timer_ticks' `unsigned long` down to a 2-byte byte array just because
    # an untyped interface-conflicts offset-placeholder happens to land 2
    # bytes later). A name is weak when EVERY source that ever named it is
    # in WEAK_ONLY_TAGS (component-public's synthetic per-component
    # fallback id, a bare name-convention decode, or an untyped
    # interface-conflicts entry with no declared type) -- see
    # `_is_weak_name`. Anything else (a real DS:-commented extern, a
    # binding, a BSS public, a *typed* interface-conflicts entry, ...)
    # still counts as strong and anchors a boundary as before.
    corroborated_offsets = sorted(
        off for off, names in table.names_by_offset.items()
        if any(not _is_weak_name(n, table, externs) for n in names))

    def next_named_offset_after(pos):
        i = bisect.bisect_right(all_offsets, pos)
        return all_offsets[i] if i < len(all_offsets) else DGROUP_LEN

    def next_corroborated_offset_after(pos):
        i = bisect.bisect_right(corroborated_offsets, pos)
        return corroborated_offsets[i] if i < len(corroborated_offsets) else DGROUP_LEN

    def _name_matches_element_type(name, elem_base, elem_is_ptr):
        entry, _, _ = resolve_historical_type({name}, externs, table, name)
        # A name declared with its OWN array brackets (even an unspecified
        # `T name[]`, e.g. `int xa[], ya[];` -- two SEPARATE arrays sharing
        # one extern statement, not one array plus a stray element name) is
        # always a genuinely separate object, never folded in as one of
        # this array's own elements just because the element type matches.
        return (entry is not None and entry['base'] == elem_base and entry['is_ptr'] == elem_is_ptr
                and not entry['dims'])

    def next_array_terminator_after(pos, elem_base, elem_is_ptr):
        """For an undimensioned array being reshaped (element type
        elem_base/elem_is_ptr): the next offset holding a name that
        actually terminates the array, per supervisor decision ("weak
        names do not terminate spans"). A name there does NOT terminate
        the array -- it becomes an element alias instead -- when it is
        either weak (`_is_weak_name`: name-convention/component-public/
        untyped-interface-conflicts is its only evidence) or genuinely
        declared with the SAME type as the array's own element (e.g.
        `int g8bec;` inside `int xa[]`, or `char far *t4;` inside `char
        far *gbfee[]`) -- only a name with real, DIFFERENTLY-typed
        evidence still ends the span."""
        off = pos + 1
        while off < DGROUP_LEN:
            for n in table.names_at(off):
                if _is_weak_name(n, table, externs):
                    continue
                if _name_matches_element_type(n, elem_base, elem_is_ptr):
                    continue
                return off
            off += 1
        return DGROUP_LEN

    overrides_by_offset = {ov['offset']: ov for ov in overrides}
    protected_starts = sorted(
        set(c['ds_offset'] for c in components
            if c['kind'] in ('code_owned', 'toolchain_opaque')
            or (c['kind'] == 'data' and c['format'] in STRUCT_SHAPED_FORMATS))
        | set(overrides_by_offset))

    def next_protected_start_after(pos):
        i = bisect.bisect_right(protected_starts, pos)
        return protected_starts[i] if i < len(protected_starts) else DGROUP_LEN

    symbols, warnings = [], []
    extra = {'segment_half': [], 'interior_alias': [], 'storage_alias': [], 'porting_notes': [],
             'floor_arrays': [], 'ceil_arrays': [], 'short_aliases': []}
    cursor = 0
    while cursor < DGROUP_LEN:
        component = comp_owner_at.get(cursor) if cursor < DATA_LEN else None

        # -- manual overrides: win over every automatic rule, claimed
        # whole, suppressing every component/symbol otherwise generated in
        # [offset, offset+length) -- see tools/portable/datagen_overrides.json.
        if cursor in overrides_by_offset:
            ov = overrides_by_offset[cursor]

            def _collect_refs(abs_lo, length):
                refs = []
                for c in components:
                    if c['kind'] != 'data':
                        continue
                    c_lo, c_hi = c['ds_offset'], c['ds_offset'] + c['length']
                    if c_hi <= abs_lo or c_lo >= abs_lo + length:
                        continue
                    for r in c.get('refs', ()):
                        abs_off = c['ds_offset'] + r['offset']
                        if abs_lo <= abs_off < abs_lo + length:
                            refs.append({**r, 'offset': abs_off - abs_lo})
                return refs

            section = 'data' if cursor < DATA_LEN else 'bss'
            if ov.get('aggregate'):
                # An "aggregate" override: the historical code treats this
                # WHOLE span as one memory block (`setmem`/`movmem` over
                # its full length -- e.g. GAME.C's `movmem(ui_gfx_shadow_a,
                # g4374, 0x2750)`), so it must stay ONE object in the
                # port too, or a whole-block copy/zero overruns into (or
                # falls short of) whatever separate objects the ordinary
                # symbol-driven walk would have carved it into. Emitted as
                # one raw `uint8_t NAME[LEN]`; every historically-named
                # symbol that used to live inside it becomes a typed
                # POINTER macro view into that same storage (an array/
                # struct view decays like a normal pointer, `((T *)(NAME +
                # off))`; a single-byte scalar view dereferences once,
                # `(*(T *)(NAME + off))`, matching how it was actually
                # used -- `gb6cf = 0;`/`while(!gb6cf)`, not array-indexed)
                # -- never a second, overlapping definition.
                ident = c_ident(ov['name'])
                sym = _make_symbol(cursor, ov['length'], section, ov['name'], [], [ov['name']], None,
                                    'generated', f"override:{ov['name']}", 'uint8_t',
                                    'manual aggregate override (tools/portable/datagen_overrides.json)',
                                    [ov['length']], False, [], False, emit_path='flat')
                symbols.append(sym)
                for view in ov['views']:
                    c_type = view['c_type']
                    off = view['offset']
                    # A view is an array/pointer form whenever the JSON
                    # gives an EXPLICIT `count` (even 1 -- e.g. b4374: a
                    # historically-scalar byte, but the ALREADY-PORTED
                    # code indexes it as `b4374[operand]`/uses it as
                    # movmem's whole-block base, so it needs to decay like
                    # a pointer to the rest of the aggregate, not
                    # dereference to one byte); omitting `count` means a
                    # true single-value scalar (`gb6cf = 0;`), which
                    # dereferences once so plain assignment/comparison
                    # works.
                    is_array = 'count' in view
                    expr = (f'(({c_type} *)({ident} + {off}))' if is_array
                            else f'(*({c_type} *)({ident} + {off}))')
                    for name in [view['name'], *view.get('aliases', ())]:
                        extra['interior_alias'].append({
                            'name': name, 'offset': cursor + off, 'array_name': ov['name'],
                            'array_offset': cursor, 'expr': expr, 'c_type': c_type,
                        })
                cursor += ov['length']
                continue
            if ov.get('layout'):
                # A "layout" override (menu-descriptor-style): several
                # named sub-objects packed into one span whose bytes
                # cannot be exposed through a byte-offset macro at all,
                # because the portable struct's pointers are wider than
                # the historical ones -- every sub-offset after the first
                # pointer field would be wrong. Each entry becomes its OWN
                # real symbol (own refs, own emit_path), exactly as if it
                # were its own top-level override at that absolute offset;
                # any byte range no entry covers becomes an anonymous
                # `uint8_t ..._gap_XXXX[N]` so the whole span still
                # round-trips exactly.
                covered = []
                for entry in ov['layout']:
                    abs_off = cursor + entry['offset']
                    if entry.get('struct'):
                        count = entry.get('count', 1)
                        length = _override_struct_field_width(entry['struct']['fields']) * count
                        c_type = f"struct {entry['struct']['tag']}"
                        emit_path = 'override-struct'
                        entry_override = {'struct': entry['struct'], 'count': count}
                        entry_refs = _collect_refs(abs_off, length)
                        dims = []
                    elif entry.get('init') == 'codeptrs':
                        count = entry['count']
                        length = count * 2
                        c_type = 'void (*)(void)'
                        emit_path = 'flat'
                        entry_override, entry_refs = None, []
                        dims = [count]
                    else:
                        # Plain typed sub-object (no pointer fields of its
                        # own): e.g. g1684, `dos_int g1684[17]` living
                        # right after g1670 inside DATA_01129F_LEVEL_
                        # CONTROL -- reuses the SAME 'flat' emitter as an
                        # ordinary symbol-driven object, just placed
                        # explicitly instead of inferred.
                        count = entry.get('count', 1)
                        base_type, _ = _parse_override_c_type(entry['c_type'])
                        length = ELEM_SIZE_OF.get(base_type, 1) * count
                        c_type = base_type
                        emit_path = 'flat'
                        entry_override, entry_refs = None, []
                        dims = [count] if count > 1 else []
                    sym = _make_symbol(abs_off, length, section, entry['name'], [], [entry['name']],
                                        None, 'generated', f"override:{ov['name']}.{entry['name']}",
                                        c_type, f"layout entry of {ov['name']!r} "
                                        '(manual override, tools/portable/datagen_overrides.json)',
                                        dims, False, [], False, emit_path=emit_path)
                    if entry_override is not None:
                        sym['override'] = entry_override
                        sym['override_refs'] = entry_refs
                    symbols.append(sym)
                    covered.append((entry['offset'], entry['offset'] + length))
                for alias, expr in ov.get('aliases', {}).items():
                    extra['interior_alias'].append({
                        'name': alias, 'offset': cursor, 'array_name': ov['name'],
                        'array_offset': cursor, 'expr': expr, 'c_type': None,
                    })
                covered.sort()
                pos = 0
                for lo, hi in covered:
                    if lo > pos:
                        gap_name = f"{c_ident(ov['name']).lower()}_gap_{cursor + pos:04X}"
                        symbols.append(_make_symbol(cursor + pos, lo - pos, section, gap_name, [],
                                                     [gap_name], None, 'generated',
                                                     f"override:{ov['name']}.{gap_name}",
                                                     None, 'layout gap (unnamed bytes)', [], False,
                                                     [], True, emit_path='flat'))
                    pos = max(pos, hi)
                if pos < ov['length']:
                    gap_name = f"{c_ident(ov['name']).lower()}_gap_{cursor + pos:04X}"
                    symbols.append(_make_symbol(cursor + pos, ov['length'] - pos, section, gap_name,
                                                 [], [gap_name], None, 'generated',
                                                 f"override:{ov['name']}.{gap_name}",
                                                 None, 'layout gap (unnamed bytes)', [], False, [],
                                                 True, emit_path='flat'))
                cursor += ov['length']
                continue
            ov_refs = _collect_refs(cursor, ov['length'])
            if ov.get('struct'):
                c_type, dims, is_ptr, emit_path = f"struct {ov['struct']['tag']}", [], False, 'override-struct'
            else:
                base, dims = _parse_override_c_type(ov['c_type']) if ov.get('c_type') else (None, [])
                c_type, is_ptr, emit_path = base, False, 'flat'
            sym = _make_symbol(cursor, ov['length'], section, ov['name'], [], [ov['name']], None,
                                'generated', f"override:{ov['name']}", c_type,
                                'manual override (tools/portable/datagen_overrides.json)',
                                dims, is_ptr, [], c_type is None, emit_path=emit_path)
            sym['override'] = ov
            sym['override_refs'] = ov_refs
            symbols.append(sym)
            for alias, expr in ov['aliases'].items():
                extra['interior_alias'].append({
                    'name': alias, 'offset': cursor, 'array_name': ov['name'],
                    'array_offset': cursor, 'expr': expr, 'c_type': c_type,
                })
            cursor += ov['length']
            continue

        # -- protected regions: claimed as one unit regardless of naming ---
        if component is not None and component['ds_offset'] == cursor and component['kind'] == 'code_owned':
            names = table.names_at(cursor)
            if names:
                primary = _pick_primary(names, table, friendly_of_offset, cursor)
                symbols.append(_make_symbol(cursor, component['length'], 'data', primary,
                                             sorted(n for n in names if n != primary), sorted(names),
                                             None, f"ported-C:{component['code_owner']}",
                                             component['id'], None, None, [], False, [], False))
            cursor += component['length']
            continue
        if component is not None and component['ds_offset'] == cursor and component['kind'] == 'toolchain_opaque':
            names = table.names_at(cursor)
            if names:
                warnings.append(
                    f"{sorted(names)} name(s) fall at DS:{cursor:04x}, inside toolchain-opaque "
                    f"region {component['id']} ({component['classification']}); no bytes are "
                    'available for it (not in recipes/data/game-initialized.json) so nothing is '
                    'emitted -- investigate whether this name is real.')
            cursor += component['length']
            continue
        if (component is not None and component['ds_offset'] == cursor and component['kind'] == 'data'
                and component['format'] == SOUND_FORMAT):
            # Supervisor round 3: DATA_01139E_SOUND must be symbol-split
            # like a flat component, not left as one opaque struct --
            # include/SOUND.H names almost every word in it.
            split_symbols, split_warnings = _split_sound_component(
                component, table, friendly_of_offset, externs)
            symbols.extend(split_symbols)
            warnings.extend(split_warnings)
            cursor += component['length']
            continue
        if (component is not None and component['ds_offset'] == cursor and component['kind'] == 'data'
                and component['format'] == PTRREC_FORMAT):
            # Round 5: each 20-byte record in a u16-farptr-u8-farptr-u8-
            # tail8-v1 component is byte-for-byte a `struct dialog`
            # (word=kind, pointer_a=title, byte_a=sub, pointer_b=text,
            # byte_b=initial, tail[8]=cx/cy/w/lines) -- one object per
            # record, named by whatever historical symbol lands on that
            # record's own offset, else dialog_XXXX by DS offset.
            assert component['length'] % KNOWN_STRUCT_SIZES['struct dialog'] == 0
            record_size = KNOWN_STRUCT_SIZES['struct dialog']
            for rec_start in range(0, component['length'], record_size):
                abs_off = cursor + rec_start
                # The component id (a source-(v) component-public, never a
                # real historical name) is excluded here so it can never
                # win the primary slot for a record -- it is kept only as
                # an alias, and only on whichever record is actually
                # pointed at by that id (see referenced_component_ids in
                # emit_game_data).
                names = table.names_at(abs_off) - {component['id']}
                if names:
                    primary = _pick_primary(names, table, friendly_of_offset, abs_off)
                    aliases = sorted(n for n in names if n != primary)
                else:
                    primary = f'dialog_{abs_off:04X}'
                    aliases = []
                if rec_start == 0:
                    aliases.append(component['id'])
                # Source (vii): docs/current/interface-conflicts.json can
                # name a byte *inside* this record (e.g. g13b8 = DS:13B8 =
                # this record's own `.text` field at sub-offset 7) rather
                # than at the record's own base -- the record's own
                # historical `struct dialog` type wins (as rule B does for
                # any other fully-dimensioned/structured declared type),
                # and the interior name becomes a `.field` alias macro via
                # STRUCT_FIELD_LAYOUTS_FOR_ALIASING['struct dialog'].
                for sub in range(1, record_size):
                    interior_names = table.names_at(abs_off + sub) - {component['id']}
                    if not interior_names:
                        continue
                    path = _struct_field_path('struct dialog', sub)
                    expr = (f'({c_ident(primary)}{path})' if path
                            else f'(*((dos_char *)(&{c_ident(primary)}) + {sub}))')
                    for n in sorted(interior_names):
                        extra['interior_alias'].append({
                            'name': n, 'offset': abs_off + sub, 'array_name': primary,
                            'array_offset': abs_off, 'expr': expr, 'c_type': 'struct dialog',
                        })
                symbols.append(_make_symbol(abs_off, record_size, 'data', primary, sorted(aliases),
                                             sorted(names | {component['id']}) if rec_start == 0
                                             else (sorted(names) or [primary]), None, 'generated',
                                             component['id'], None, None, [], False, [], False,
                                             emit_path='ptrrec-dialog'))
            cursor += component['length']
            continue
        if (component is not None and component['ds_offset'] == cursor and component['kind'] == 'data'
                and component['format'] in STRUCT_SHAPED_FORMATS):
            names = table.names_at(cursor)
            primary = _pick_primary(names, table, friendly_of_offset, cursor) if names else component['id']
            aliases = sorted(n for n in names if n != primary)
            # DATA objects are always generated regardless of state_ownership.json
            # (that file only suppresses BSS definitions; see its own description).
            symbols.append(_make_symbol(cursor, component['length'], 'data', primary, aliases,
                                         sorted(names) or [component['id']], None, 'generated',
                                         component['id'], None, None, [], False, [], False,
                                         emit_path='struct-component'))
            cursor += component['length']
            continue

        # -- generic symbol-driven claim -------------------------------------
        names = table.names_at(cursor)
        if not names:
            if component is not None and not component_has_any_name.get(component['id'], True):
                # Nothing anywhere in this flat component is named: one
                # fallback object, named by the component id, using the
                # recipe format for a sensible default representation.
                start = max(cursor, component['ds_offset'] + component['skip'])
                end = component['ds_offset'] + component['length']
                symbols.append(_make_symbol(start, end - start, 'data', component['id'], [],
                                             [component['id']], None, 'generated', component['id'],
                                             None, None, [], False, [], True,
                                             emit_path='component-fallback'))
                cursor = end
                continue
            # A genuinely un-named stretch (e.g. DATA_00FCC2's skipped
            # leading word, or a BSS gap): emit nothing, advance.
            boundary = min(next_named_offset_after(cursor), next_protected_start_after(cursor))
            if component is not None:
                boundary = min(boundary, component['ds_offset'] + component['length'])
            cursor = max(boundary, cursor + 1)
            continue

        primary = _pick_primary(names, table, friendly_of_offset, cursor)
        aliases = sorted(n for n in names if n != primary)
        # `names` is a set; iterate `[primary] + aliases` (aliases already
        # sorted above) instead of the set directly, so which owned name
        # gets reported is deterministic -- preferring the object's own
        # canonical primary name -- rather than depending on hash-
        # randomized set iteration order (this fed a real cross-process
        # nondeterminism in the emitted `owned_by` field, e.g. `cur_idx`
        # vs. `g3902` for the same object depending on process hash seed).
        owned_name = (next((n for n in [primary] + aliases if n in ownership), None)
                      if cursor >= DATA_LEN else None)
        section = 'data' if cursor < DATA_LEN else 'bss'
        emit_path_override = None
        ceil_overshoot = False

        chosen, chosen_name, alias_type_reports = resolve_historical_type(names, externs, table, primary)
        c_type = dims = is_ptr = type_note = None
        had_array = False
        untyped = True
        tr = typed_reserves.get(cursor) if section == 'bss' else None
        # Rule G: jmp_buf is a more structured, more specific type than a
        # generic char/uint8 array an extern re-declaration might give the
        # same object (e.g. BOARD.C's `char game_abort_jmpbuf[]` vs.
        # production-plan.json's typed_reserves 'jmp_buf' for the same
        # offset) -- prefer it outright rather than by width/rank.
        if tr is not None and tr['base'] == 'jmp_buf':
            c_type, type_note, dims, is_ptr = 'jmp_buf', None, [], False
            untyped = False
            type_note = f"[typed_reserves {tr['block']}: {tr['source_type']}] (rule G: host jmp_buf)"
        elif chosen is not None:
            mapped, note = resolve_primitive(chosen['base'])
            is_ptr = chosen['is_ptr'] or chosen['base'] == 'void (*)(void)'
            if chosen['base'] == 'void (*)(void)' and not chosen['dims']:
                # Rule A/H: DOS plumbing (interrupt vectors etc.) with no
                # portable meaning -- still emitted, as a plain `void *`,
                # not a function-pointer type, so ported units compile.
                # Only for a SCALAR function pointer, though -- an ARRAY of
                # them (`void (*g12a1[])(void)`, point 2 "code-pointer
                # tables") keeps its real historical type so it goes
                # through emit_generic_flat_object's dedicated code-
                # pointer-table resolution instead of this DOS-plumbing
                # fallback.
                mapped, is_ptr = 'void', True
            c_type, type_note = mapped, note
            dims = [v for v in (_c_int_literal(d) for d in chosen['dims']) if v is not None]
            had_array = bool(chosen['dims'])  # even a single unspecified `[]` counts
            untyped = mapped is None
            type_note = (type_note or '') + f" [extern {chosen_name} @ {chosen['file']}:{chosen['line']}: " \
                                             f"{chosen['type_text']}]"
        elif tr is not None:
            mapped, note = resolve_primitive(tr['base'])
            c_type, type_note, dims, is_ptr = mapped, note, tr['dims'], tr['is_ptr']
            untyped = mapped is None
            type_note = (type_note or '') + f" [typed_reserves {tr['block']}: {tr['source_type']}]"
        elif any(_is_s_string_name(n) for n in names):
            # No extern/typed_reserves evidence, but an 's'-prefixed
            # name-convention name (source vi) claims this offset: treated
            # as a DATA string (dos_char[]) rather than a plain byte array.
            # `names` is a set; pick deterministically (sorted) rather than
            # depending on hash-randomized set iteration order for which
            # name ends up quoted in the generated comment.
            s_name = next(n for n in sorted(names) if _is_s_string_name(n))
            c_type, dims, is_ptr, untyped = 'dos_char', [], False, False
            type_note = f"[name-convention {s_name}: 's'-prefixed names are DATA strings]"

        span = tr['span'] if c_type == 'jmp_buf' else (_historical_span(chosen) if chosen is not None else None)
        if span is not None:
            # A fully-dimensioned historical type is authoritative -- BUT
            # clip it against the next actually-named offset/protected
            # region as a safety net: a locally mis-declared array (wrong
            # dimension in one .C file) must never silently swallow other,
            # independently-named real objects that follow it.
            safe_boundary = min(next_corroborated_offset_after(cursor), next_protected_start_after(cursor))
            if span > safe_boundary - cursor:
                # Rule A: a 4-byte far pointer (or interrupt-vector function
                # pointer) whose only "conflict" is a name sitting exactly
                # 2 bytes in is that name naming the pointer's own
                # historical SEGMENT half, not a separate object -- absorb
                # it instead of clipping the pointer down to nothing.
                seg_offset = cursor + 2
                is_far_ptr_span4 = (is_ptr or chosen['base'] == 'void (*)(void)') and span == 4 \
                    and not chosen.get('is_near', False)
                interior_entries = None
                if not is_far_ptr_span4 and dims and c_type in KNOWN_STRUCT_SIZES:
                    lo = bisect.bisect_right(corroborated_offsets, cursor)
                    hi = bisect.bisect_left(corroborated_offsets, cursor + span)
                    interior_entries = [(o, sorted(table.names_at(o) - {primary}))
                                         for o in corroborated_offsets[lo:hi]]
                    interior_entries = [(o, ns) for o, ns in interior_entries if ns]
                if is_far_ptr_span4 and safe_boundary == seg_offset:
                    seg_names = sorted(table.names_at(seg_offset))
                    extra['segment_half'].append({
                        'pointer_offset': cursor, 'pointer_name': primary,
                        'segment_offset': seg_offset, 'segment_names': seg_names,
                    })
                elif interior_entries:
                    # Rule B: the fully-dimensioned declared type wins; each
                    # interior name becomes an alias EXPRESSION macro
                    # (a precise `.field[i]` path when the struct's field
                    # layout is known, else a raw byte-offset cast) instead
                    # of clipping the array down.
                    elem_bytes = span // dims[0] if dims and dims[0] else span
                    for offset_here, names_here in interior_entries:
                        sub = offset_here - cursor
                        idx, field_sub = divmod(sub, elem_bytes)
                        path = _struct_field_path(c_type, field_sub)
                        expr = (f'({primary}[{idx}]{path})' if path
                                else f'(*((dos_char *)({primary}) + {sub}))')
                        for n in names_here:
                            extra['interior_alias'].append({
                                'name': n, 'offset': offset_here, 'array_name': primary,
                                'array_offset': cursor, 'expr': expr, 'c_type': c_type,
                            })
                elif len(dims) > 1 and c_type is not None:
                    # Rule C: a multi-dimensional array that overruns the
                    # next real symbol -- clip the FIRST (outermost)
                    # dimension and keep the row stride (the code indexes
                    # rows by that stride, so it is what matters, not the
                    # historically-declared row count).
                    row_elems = 1
                    for d in dims[1:]:
                        row_elems *= d
                    elem_size = historical_elem_size(chosen['base'], is_ptr,
                                                      chosen.get('is_near', False), c_type)
                    row_bytes = row_elems * elem_size
                    available = safe_boundary - cursor
                    new_outer = available // row_bytes
                    if row_bytes and new_outer >= 1:
                        extra['porting_notes'].append(
                            f"{primary!r} at DS:{cursor:04x}: declared type {chosen['type_text']!r} "
                            f"({chosen['file']}:{chosen['line']}) implies {dims[0]} rows but only "
                            f'{available} bytes are available before the next real symbol; rule C '
                            f'clips the outer dimension to {new_outer} rows (stride '
                            f'{row_bytes} bytes unchanged, indexing arithmetic is unaffected).')
                        dims = [new_outer] + dims[1:]
                        span = new_outer * row_bytes
                    else:
                        warnings.append(
                            f"{primary!r} at DS:{cursor:04x}: declared type {chosen['type_text']!r} "
                            f"({chosen['file']}:{chosen['line']}) implies {span} bytes with a "
                            f'{row_bytes}-byte row stride, which does not even fit once before the '
                            f'next real symbol at DS:{safe_boundary:04x} ({available} bytes away); '
                            'falling back to an untyped byte array.')
                        span = max(available, 1)
                        c_type, dims, is_ptr, untyped = None, [], False, True
                else:
                    warnings.append(
                        f"{primary!r} at DS:{cursor:04x}: declared type {chosen['type_text']!r} "
                        f"({chosen['file']}:{chosen['line']}) implies {span} bytes, which reaches "
                        f"past the next named offset/protected region at DS:{safe_boundary:04x} "
                        f'({safe_boundary - cursor} bytes away); the declared type is almost '
                        'certainly wrong here -- falling back to an untyped byte array of the '
                        'clipped size instead of swallowing another real object.')
                    span = max(safe_boundary - cursor, 1)
                    c_type, dims, is_ptr, untyped = None, [], False, True
                    type_note = f"declared type conflicted with a nearer symbol at DS:{safe_boundary:04x}"
            size = span
        elif chosen is None and section == 'bss' and c_type is not None and dims:
            # typed_reserves already gave an authoritative span (see
            # load_bss_typed_reserves' own count*element_bytes check).
            # Gated on `chosen is None`: when an extern DID resolve (chosen
            # is not None) but its span came back unspecified (the branch
            # above only sets `span is not None` for a FULLY dimensioned
            # type), `dims` here holds only the inner dimension(s) of a
            # `T name[][N]`-style declarator and still needs the reshape
            # below to fill in the missing outer count from the measured
            # span -- taking this shortcut instead would silently keep the
            # un-reshaped inner-only dims (see gc6c3: `unsigned char
            # gc6c3[][24]` was emitted as `dos_uchar gc6c3[24]`, dropping
            # the span-derived outer dimension entirely).
            size = typed_reserves[cursor]['span']
        else:
            # A partially-dimensioned array's reshape runs to the next
            # STRONG symbol -- one that is neither weak (name-convention/
            # component-public/untyped-interface-conflicts) nor typed
            # identically to the array's own element (an interior name
            # declared with the SAME element type is one of the array's
            # own elements, not a competing object -- see
            # next_array_terminator_after / "weak names do not terminate
            # spans"); anything with no resolved array type at all should
            # stop at ANY named offset, including a weak one, to stay
            # conservative.
            if had_array and c_type is not None:
                boundary = min(next_array_terminator_after(cursor, chosen['base'], is_ptr),
                                next_protected_start_after(cursor))
                # A code-pointer table's (point 2) own recipe component
                # span is real data provenance, not just a naming
                # convenience -- it must never absorb a NEIGHBORING
                # component's unrelated bytes just because nothing strong
                # names the offset right after it (u16le-table-v1 isn't a
                # STRUCT_SHAPED_FORMATS protected component, so
                # next_protected_start_after doesn't already cover this).
                # Scoped to func-ptr arrays only: an ordinary data array's
                # sanctioned overshoot into a neighboring PROTECTED
                # component (rule E: ga5e's last record legitimately
                # overlaps gb2a's leading bytes) must keep working exactly
                # as before -- next_protected_start_after already bounds
                # that case correctly on its own.
                if component is not None and chosen['base'] == 'void (*)(void)':
                    boundary = min(boundary, component['ds_offset'] + component['length'])
            else:
                boundary = min(next_named_offset_after(cursor), next_protected_start_after(cursor))
            size = max(boundary - cursor, 1)
            if had_array and c_type is not None:
                # An array with an unspecified outer dimension (possibly
                # its *only* dimension, e.g. `long ga52[]`): reshape the
                # measured span using whatever inner dims ARE explicit.
                if chosen['base'] == 'void (*)(void)':
                    # Point 2 "code-pointer tables": an ARRAY of function
                    # pointers is a compact-model code address (this
                    # program's own single _TEXT segment) -- 2 bytes, not
                    # historical_elem_size's far-pointer default (correct
                    # for rule A/H's SCALAR interrupt-vector case, which
                    # never reaches this had_array branch at all).
                    elem_size = 2
                else:
                    elem_size = historical_elem_size(chosen['base'], is_ptr,
                                                      chosen.get('is_near', False), c_type)
                inner = 1
                for d in dims:
                    inner *= d
                row_bytes = inner * elem_size
                if row_bytes and size % row_bytes == 0:
                    dims = [size // row_bytes] + dims
                    if primary == 'g13ef' and dims == [1]:
                        # Rule D special case: SLOTMENU.C declares a scalar
                        # `int g13ef;`; LEVEL.C's `int g13ef[]` is only
                        # ever indexed at [0] -- it names the same single
                        # int, not a large array. The general weak-name-
                        # aware boundary now measures this correctly on
                        # its own (no longer needs its own dedicated
                        # branch below), but the porting note is still
                        # worth keeping for whoever ports SLOTMENU.C/
                        # LEVEL.C.
                        extra['porting_notes'].append(
                            'SLOTMENU.C uses the scalar spelling: it must be ported as g13ef[0] '
                            "(LEVEL.C's `int g13ef[]` is only ever indexed at [0]).")
                    # Every name strictly inside [cursor, cursor+size) is,
                    # by next_array_terminator_after's own contract, either
                    # weak or same-element-typed -- safe to alias to its
                    # element (or, if it lands off an element boundary, a
                    # raw byte-offset cast) without re-checking.
                    off = cursor + 1
                    while off < cursor + size:
                        for n in sorted(table.names_at(off)):
                            sub = off - cursor
                            if row_bytes and sub % row_bytes == 0:
                                elem_expr = f'({primary}[{sub // row_bytes}])'
                            else:
                                elem_expr = f'(*((dos_char *)({primary}) + {sub}))'
                            extra['interior_alias'].append({
                                'name': n, 'offset': off, 'array_name': primary,
                                'array_offset': cursor, 'expr': elem_expr, 'c_type': c_type,
                            })
                        off += 1
                elif primary in STORAGE_ALIAS_UNIONS and next_protected_start_after(cursor) == boundary \
                        and not (comp_owner_at.get(boundary) or {}).get('refs'):
                    # Rule F: real historical aliasing of the same storage
                    # as a neighboring struct-shaped component -- emit ONE
                    # object for the union of both spans (raw bytes) and
                    # view each historical name through a macro cast
                    # instead of a second, overlapping definition.
                    union_component = comp_owner_at[boundary]
                    union_name = STORAGE_ALIAS_UNIONS[primary]
                    union_size = (boundary - cursor) + union_component['length']
                    extra['storage_alias'].append({
                        'union_name': union_name, 'offset': cursor, 'size': union_size,
                        'members': [primary, union_component['id']],
                        'union_component_id': union_component['id'],
                    })
                    size = union_size
                    names = names | {union_name, union_component['id']}
                    primary, c_type, dims, is_ptr, untyped = union_name, None, [], False, True
                    aliases = []  # the original names become macro casts, not plain #defines
                    emit_path_override = 'storage-union'
                elif primary == 'g13ef':
                    # Rule D special case: SLOTMENU.C declares a scalar
                    # `int g13ef;`; LEVEL.C's `int g13ef[]` is only ever
                    # indexed at [0] -- it names the same single int, not a
                    # large array, so the wide measured span is unrelated
                    # bytes, not part of this object.
                    dims = [1] + dims
                    size = row_bytes
                    extra['porting_notes'].append(
                        'SLOTMENU.C uses the scalar spelling: it must be ported as g13ef[0] '
                        "(LEVEL.C's `int g13ef[]` is only ever indexed at [0]; the historical "
                        'extern inventory would otherwise measure a much wider, unrelated span).')
                elif row_bytes and c_type in KNOWN_STRUCT_SIZES:
                    # Rule E: a struct array whose last record is only
                    # partially covered by the measured span -- the
                    # historical code only ever reads whole records, so
                    # round UP and document the resulting overlap with
                    # whatever object follows (its own, independent copy of
                    # the same DATA-image bytes -- this can legitimately
                    # overlap a protected struct-component's leading bytes,
                    # e.g. ga5e's last 6 bytes duplicate gb2a's first 6;
                    # `cursor_advance` below still stops at that
                    # component's own base so it is claimed atomically
                    # right after, undiminished).
                    count = -(-size // row_bytes)  # ceil
                    new_size = count * row_bytes
                    dims = [count] + dims
                    extra['ceil_arrays'].append({
                        'name': primary, 'offset': cursor, 'measured': size, 'elem_bytes': row_bytes,
                        'count': count, 'new_size': new_size, 'overlap': new_size - size,
                        'overlap_at': cursor + size, 'c_type': c_type,
                    })
                    size = new_size
                    ceil_overshoot = True
                elif row_bytes:
                    # Rule D: floor -- keep the type, leave the incomplete
                    # trailing element's bytes unnamed/unclaimed (the
                    # historical code never reads a partial element).
                    count = size // row_bytes
                    leftover = size - count * row_bytes
                    dims = [count] + dims
                    extra['floor_arrays'].append({
                        'name': primary, 'offset': cursor, 'measured': size, 'elem_bytes': row_bytes,
                        'count': count, 'leftover': leftover, 'c_type': c_type,
                    })
                    size = count * row_bytes
                else:
                    c_type, dims, is_ptr, untyped = None, [], False, True

        definition_site = 'subsystem-owned' if owned_name else 'generated'
        symbols.append(_make_symbol(cursor, size, section, primary, aliases, sorted(names),
                                     owned_name, definition_site, component['id'] if component else None,
                                     c_type, type_note, dims, is_ptr, alias_type_reports, untyped,
                                     emit_path=emit_path_override or 'flat'))
        # Normally the object's own span IS how far the walk advances --
        # except a rule-E ceil can overshoot into a protected struct-
        # component's own base (sanctioned: two independent objects with
        # their own copies of the same overlapping DATA-image bytes). The
        # outer walk must still stop AT that base, never skip past it, so
        # the struct-component is claimed atomically right after, in full.
        if ceil_overshoot:
            cursor = min(cursor + size, next_protected_start_after(cursor))
        else:
            cursor += size

    return symbols, warnings, extra


def _make_symbol(offset, size, section, primary, aliases, names, owned_by, definition_site,
                  component_id, c_type, type_note, dims, is_ptr, alias_type_reports, untyped,
                  emit_path='flat'):
    """`emit_path` (DATA only) tells emit_game_data() which emitter owns
    this symbol: 'struct-component' (a struct-shaped recipe component,
    claimed whole -- describe_and_emit_component's TYPED/SOUND/PTRREC/
    INSTR branches), 'component-fallback' (a whole anonymous component,
    format-aware default -- describe_and_emit_component's flat-format
    branches), or 'flat' (a plain symbol-driven span -- emit_generic_flat_object)."""
    conflict = any(not r['compatible'] for r in alias_type_reports)
    return {
        'offset': offset, 'size': size, 'section': section, 'primary': primary,
        'aliases': aliases, 'names': names, 'owned_by': owned_by,
        'definition_site': definition_site, 'component_id': component_id,
        'c_type': c_type, 'type_note': type_note, 'dims': dims, 'is_ptr': is_ptr,
        'untyped': untyped, 'conflict': conflict, 'emit_path': emit_path,
        'alias_type_reports': [
            {'name': r['name'], 'type_text': r['entry']['type_text'], 'file': r['entry']['file'],
             'line': r['entry']['line'], 'compatible': r['compatible']}
            for r in alias_type_reports],
    }


# ---------------------------------------------------------------------------
# Step C -- emit portable/generated/game_data.[ch] and game_state.[ch]
# ---------------------------------------------------------------------------

def c_ident(name):
    ident = re.sub(r'[^0-9A-Za-z_]', '_', name)
    if ident[:1].isdigit():
        ident = '_' + ident
    return ident


def c_escape_string(data):
    """Escape `data` (bytes, no embedded NUL expected for ascii-nul-v1 minus
    its own terminator) as a C string-literal body.  Every non-printable or
    quote/backslash byte becomes a *fixed-width* 3-digit octal escape, which
    C always consumes exactly 3 digits of -- never ambiguous with a
    following literal digit, unlike \\xHH hex escapes."""
    out = []
    for b in data:
        ch = chr(b)
        if ch == '\\':
            out.append('\\\\')
        elif ch == '"':
            out.append('\\"')
        elif 0x20 <= b < 0x7f:
            out.append(ch)
        else:
            out.append('\\%03o' % b)
    return ''.join(out)


def c_int_array(values, per_line=12):
    rows = []
    for i in range(0, len(values), per_line):
        rows.append(', '.join(str(v) for v in values[i:i + per_line]))
    return '{\n    ' + ',\n    '.join(rows) + '\n}'


class PointerResolver:
    """Resolves a recipe pointer target ('DATA_xxxx...' component id,
    a component-local public name, or the literal 'GAME_BSS') plus an
    addend to a concrete (symbol, local_offset_within_symbol) pair, so the
    emitter can print `&symbol[local_offset]` / `symbol` (array decay)."""

    def __init__(self, table, symbols):
        self.table = table
        self.by_offset = sorted(symbols, key=lambda s: s['offset'])
        self._starts = [s['offset'] for s in self.by_offset]

    def _symbol_at(self, abs_offset):
        import bisect
        i = bisect.bisect_right(self._starts, abs_offset) - 1
        if i < 0:
            raise ValueError(f'no symbol covers DGROUP offset {abs_offset:#x}')
        sym = self.by_offset[i]
        if not (sym['offset'] <= abs_offset < sym['offset'] + sym['size']):
            raise ValueError(f'DGROUP offset {abs_offset:#x} falls in an unclaimed gap')
        return sym, abs_offset - sym['offset']

    def resolve(self, target, addend):
        base = DATA_LEN if target == 'GAME_BSS' else self.table.offset_by_name.get(target)
        if base is None:
            raise ValueError(f'unresolved pointer target {target!r}')
        return self._symbol_at(base + addend)

    def expr(self, target, addend, emitted_kind_by_symbol):
        sym, local = self.resolve(target, addend)
        name = c_ident(sym['primary'])
        kind = emitted_kind_by_symbol.get(sym['primary'])
        if sym.get('owned_by'):
            note = f'/* WARNING: pointer target {name} is subsystem-owned; verify the type below matches its real declaration */ '
        else:
            note = ''
        if local == 0:
            if kind in ('array', 'char_array', 'record_array'):
                return note + name  # array-to-pointer decay
            return note + f'&{name}'
        if kind == 'struct':
            fields = _STRUCT_FIELD_REGISTRY.get(sym['primary'], [])
            for start, fname, elem_size, length in fields:
                if start <= local < start + length:
                    rel = local - start
                    if rel == 0:
                        return note + f'&{name}.{fname}'
                    if rel % elem_size:
                        raise ValueError(
                            f'pointer target {target!r}+{addend} lands {rel} bytes into field '
                            f'{fname} of struct {name}, not on an element boundary '
                            f'(element size {elem_size})')
                    return note + f'&{name}.{fname}[{rel // elem_size}]'
            raise ValueError(f'pointer target {target!r}+{addend} lands {local} bytes inside '
                              f'struct {name}, which has no field covering that offset')
        if kind not in ('array', 'char_array', 'record_array'):
            raise ValueError(f'pointer target {target!r}+{addend} lands {local} bytes inside '
                              f'{name}, which is not emitted as a byte-indexable array')
        return note + f'&{name}[{local}]'


def pointer_field_c_type(target_format):
    if target_format in ('ascii-nul-v1', 'ascii-v1'):
        return 'dos_char *'
    if target_format == 'u16le-table-v1':
        return 'dos_uint *'
    return 'void *'


def describe_and_emit_component(component, image, table, resolver, comp_by_id, ident):
    """Return an EmittedObject-shaped dict for one 'data'-kind component:
    {c_type, emit_kind, header_extra (struct typedef lines), decl, defn,
     verify_bytes}.  `emit_kind` drives PointerResolver's array-decay choice
     and is one of: char_array (dos_char[N]), array (numeric element array,
     decays like a char_array for &-purposes), record_array, struct, scalar.
    """
    fmt = component['format']
    ds0, length = component['ds_offset'], component['length']
    raw = image[ds0:ds0 + length]

    if fmt == 'ascii-nul-v1':
        # The array is exactly len(text)+1, so the string literal's implicit
        # NUL fills it exactly -- no extra byte, no truncation.
        literal = c_escape_string(raw[:-1])
        defn = f'dos_char {ident}[{length}] = "{literal}";\n'
        return {'c_type': f'dos_char[{length}]', 'emit_kind': 'char_array',
                'header_extra': [], 'decl': f'extern dos_char {ident}[{length}];',
                'defn': defn, 'verify_bytes': raw}

    if fmt == 'ascii-v1':
        # Unterminated: a `= "..."` string literal would need the array
        # exactly string-length sized with no room for the literal's own
        # implicit NUL, which is legal C but MSVC /W4 flags as C4295
        # ("array too small to include a terminating null"); use a byte
        # initializer instead so the text is still readable in the source
        # but nothing implies a NUL that was never historically there.
        literal = c_escape_string(raw)
        signed_values = [b - 256 if b > 127 else b for b in raw]
        defn = (f'dos_char {ident}[{length}] = /* "{literal}" (not NUL-terminated) */ '
                f'{c_int_array(signed_values)};\n')
        return {'c_type': f'dos_char[{length}]', 'emit_kind': 'char_array',
                'header_extra': [], 'decl': f'extern dos_char {ident}[{length}];',
                'defn': defn, 'verify_bytes': raw}

    if fmt == 'u16le-table-v1':
        values = [int.from_bytes(raw[i:i + 2], 'little') for i in range(0, len(raw), 2)]
        n = len(values)
        defn = f'dos_uint {ident}[{n}] = {c_int_array(values)};\n'
        return {'c_type': f'dos_uint[{n}]', 'emit_kind': 'array',
                'header_extra': [], 'decl': f'extern dos_uint {ident}[{n}];',
                'defn': defn, 'verify_bytes': raw}

    if fmt == 'zero-pad-v1':
        defn = f'dos_uchar {ident}[{length}] = {{0}};\n'
        return {'c_type': f'dos_uchar[{length}]', 'emit_kind': 'array',
                'header_extra': [], 'decl': f'extern dos_uchar {ident}[{length}];',
                'defn': defn, 'verify_bytes': raw}

    if fmt == 'fixed-records-v1':
        # record_size groups the bytes but implies no historical scalar
        # width; emit plain uint8_t per the "type could not be determined"
        # fallback rule instead of dressing this up with a dos_* alias.
        values = list(raw)
        defn = f'uint8_t {ident}[{length}] = {c_int_array(values)};\n'
        return {'c_type': f'uint8_t[{length}]', 'emit_kind': 'array',
                'header_extra': [], 'decl': f'extern uint8_t {ident}[{length}];',
                'defn': defn, 'verify_bytes': raw, 'untyped': True,
                'untyped_note': 'fixed-records-v1 record structure not modeled; raw bytes only'}

    if fmt == 'dac6-rgb256-v1':
        rows = [list(raw[i:i + 3]) for i in range(0, 768, 3)]
        body = ',\n    '.join('{' + ', '.join(str(c) for c in rgb) + '}' for rgb in rows)
        defn = f'dos_uchar {ident}[256][3] = {{\n    {body}\n}};\n'
        return {'c_type': 'dos_uchar[256][3]', 'emit_kind': 'array',
                'header_extra': [], 'decl': f'extern dos_uchar {ident}[256][3];',
                'defn': defn, 'verify_bytes': raw}

    if fmt == TYPED_FORMAT:
        return _emit_typed_data(component, raw, ident, resolver, comp_by_id)

    if fmt == PTRREC_FORMAT:
        return _emit_pointer_records(component, raw, ident, resolver, comp_by_id)

    if fmt == SOUND_FORMAT:
        return _emit_sound_data(component, raw, ident, resolver, comp_by_id)

    if fmt == INSTR_FORMAT:
        return _emit_sound_instruments(component, raw, ident)

    raise ValueError(f'no emitter for format {fmt!r}')


def _is_c_string_like(data):
    if not data or data[-1] != 0 or 0 in data[:-1]:
        return False
    return all(0x20 <= b < 0x7f or b in (0x0a, 0x0d, 0x09) for b in data[:-1])


def _nested_int_array(values, dims, signed_wrap=None):
    """Render `values` (flat, row-major) shaped by `dims` (possibly empty ->
    a flat list) as a nested C brace initializer."""
    if not dims:
        vals = [signed_wrap(v) for v in values] if signed_wrap else values
        return c_int_array(vals)
    outer, *rest = dims
    if not rest:
        vals = [signed_wrap(v) for v in values] if signed_wrap else values
        return c_int_array(vals)
    stride = 1
    for d in rest:
        stride *= d
    rows = [_nested_int_array(values[i * stride:(i + 1) * stride], rest, signed_wrap)
            for i in range(outer)]
    return '{\n    ' + ',\n    '.join(rows) + '\n}'


_TEXT_OFFSET_TO_FUNCTION_CACHE = None


def text_offset_to_function_name():
    """_TEXT-relative code offset (layout/manifest.json's `frames['_TEXT']`
    coordinate, matching docs/current/interface-conflicts.json's OMF
    evidence and DATA-embedded code-pointer tables like ROUNDEND.C's
    g12a1[]) -> the ported C function's name (no leading '_').

    layout/production-plan.json's modules give a FILE-byte `start` for
    each compiled module and, per module, a `publics` list of
    {owner, symbol, offset} where `offset` is relative to that module's
    OWN start -- so a public's file address is `module['start'] +
    public['offset']`, and its _TEXT-frame offset is that minus the
    512-byte DOS EXE header (the same `+ 512` used elsewhere for the
    DGROUP frame; manifest.json's `frames['_TEXT']` is 0, so this is the
    whole adjustment). Verified against all 6 of g12a1's entries (e.g.
    0x9871 -> `_roundend_draw_marker`, confirmed via module C_984C_9871).
    """
    global _TEXT_OFFSET_TO_FUNCTION_CACHE
    if _TEXT_OFFSET_TO_FUNCTION_CACHE is None:
        plan = read_json(PLAN_PATH)
        out = {}
        for m in plan['modules']:
            base = m['start'] - 512
            for p in m.get('publics', ()):
                symbol = p.get('symbol')
                if not symbol:
                    continue
                name = symbol[1:] if symbol.startswith('_') else symbol
                out[base + p['offset']] = name
        _TEXT_OFFSET_TO_FUNCTION_CACHE = out
    return _TEXT_OFFSET_TO_FUNCTION_CACHE


def emit_generic_flat_object(symbol, image, ident):
    """Emit a symbol-driven object that is neither a whole anonymous
    component (format-aware fallback, see describe_and_emit_component) nor
    a struct-shaped component (typed-data/sound-data/sound-instruments/
    pointer-records, see the _emit_* functions below): a plain historical-
    type object (scalar/array/pointer of dos_char/dos_uchar/dos_int/
    dos_uint/dos_long/dos_ulong, or a pointer-free struct from
    game_structs.h) whose bytes come straight from the DATA image, or the
    `uint8_t name[N]` fallback when no historical type could be resolved.
    DATA pointer fields are out of scope here (only struct-shaped
    components carry symbolic pointer refs; a flat object typed as a
    pointer has no way to resolve a target, so a non-NULL one falls back
    to a documented raw-byte array instead of guessing).
    """
    raw = image[symbol['offset']:symbol['offset'] + symbol['size']]
    c_type, dims, is_ptr = symbol['c_type'], symbol['dims'], symbol['is_ptr']

    if symbol['untyped'] or c_type is None:
        values = list(raw)
        n = len(values)
        defn = f'uint8_t {ident}[{n}] = {c_int_array(values)};\n'
        return {'c_type': f'uint8_t[{n}]', 'emit_kind': 'array', 'header_extra': [],
                'decl': f'extern uint8_t {ident}[{n}];', 'defn': defn, 'verify_bytes': raw}

    if c_type == 'void (*)(void)' and dims:
        # An ARRAY of function pointers (point 2, "code-pointer tables"):
        # each 2-byte historical element (compact model: code near, see
        # historical_elem_size) is a raw _TEXT-relative code offset, not a
        # DATA-side recipe ref -- resolve it against every ported
        # function's own entry offset (text_offset_to_function_name())
        # instead of the DATA-pointer machinery (PointerResolver), which
        # has no notion of code addresses at all.
        count = 1
        for d in dims:
            count *= d
        text_map = text_offset_to_function_name()
        values, missing = [], []
        for i in range(count):
            off = int.from_bytes(raw[i * 2:i * 2 + 2], 'little')
            fn = text_map.get(off)
            if fn is None:
                missing.append((i, off))
                values.append(f'(void (*)(void))0 /* unresolved _TEXT:{off:04x}, no public found */')
            else:
                values.append(f'(void (*)(void)){fn}')
        dims_suffix = ''.join(f'[{d}]' for d in dims)
        rows = ',\n    '.join(values)
        defn = f'void (*{ident}{dims_suffix})(void) = {{\n    {rows}\n}};\n'
        note = (f' /* {len(missing)} unresolved code offset(s): '
                f'{", ".join(f"{o:04x}" for _, o in missing)} */' if missing else '')
        return {'c_type': f'void (*)(void){dims_suffix}', 'emit_kind': 'array',
                'header_extra': [f'/* code-pointer table -- resolved via layout/production-plan.json '
                                  f'publics{note} */'],
                'decl': f'extern void (*{ident}{dims_suffix})(void);', 'defn': defn, 'verify_bytes': raw}

    if is_ptr or c_type == 'void (*)(void)':
        if all(b == 0 for b in raw):
            ptr_type = 'void (*)(void)' if c_type == 'void (*)(void)' else f'{c_type} *'
            sep = '' if ptr_type.endswith('*') else ' '
            dims_suffix = ''.join(f'[{d}]' for d in dims)
            init = '{0}' if dims else '0'
            decl = f'extern {ptr_type}{sep}{ident}{dims_suffix};'
            defn = f'{ptr_type}{sep}{ident}{dims_suffix} = {init};\n'
            return {'c_type': f'{ptr_type}{dims_suffix}', 'emit_kind': 'array', 'header_extra': [],
                    'decl': decl, 'defn': defn, 'verify_bytes': raw}
        # A non-NULL historically-typed pointer with no ref to resolve it
        # against: safer to surface the raw bytes than guess a target.
        values = list(raw)
        n = len(values)
        defn = (f'/* historical type was a pointer ({c_type}); no symbolic target is '
                f'available outside a struct-shaped component, so the raw bytes are kept */\n'
                f'uint8_t {ident}[{n}] = {c_int_array(values)};\n')
        return {'c_type': f'uint8_t[{n}]', 'emit_kind': 'array', 'header_extra': [],
                'decl': f'extern uint8_t {ident}[{n}];', 'defn': defn, 'verify_bytes': raw,
                'untyped': True, 'untyped_note': f'historical pointer type {c_type} not resolved'}

    if c_type in SINGLE_ARRAY_FIELD_STRUCTS:
        # This struct (per portable/include/game_structs.h) is exactly one
        # byte array field wrapped in a struct, e.g. `struct ga5e_entry {
        # dos_char b[0x23]; };` -- a `{ {bytes} }` initializer is always
        # correct for it (no field layout ambiguity, no pointer fields).
        field = SINGLE_ARRAY_FIELD_STRUCTS[c_type]
        elem = KNOWN_STRUCT_SIZES[c_type]
        count = max(len(raw) // elem, 1)
        dims_suffix = f'[{count}]' if not dims else ''.join(f'[{d}]' for d in dims)
        rows = ',\n    '.join('{ .' + field + ' = ' + c_int_array(list(raw[i * elem:(i + 1) * elem]))
                              + ' }' for i in range(count))
        defn = f'{c_type} {ident}{dims_suffix} = {{\n    {rows}\n}};\n'
        return {'c_type': f'{c_type}{dims_suffix}', 'emit_kind': 'array', 'header_extra': [],
                'decl': f'extern {c_type} {ident}{dims_suffix};', 'defn': defn, 'verify_bytes': raw}

    if c_type in STRUCT_INIT_FIELD_WIDTHS:
        # A pointer-free struct with more than one field, but every field's
        # byte width is known (see STRUCT_INIT_FIELD_WIDTHS) -- a flat,
        # non-designated positional initializer per record, relying on
        # ISO C brace elision to fill any nested aggregate field, is always
        # byte-exact and needs no field *names* (point 3: real struct type,
        # not a uint8_t[] fallback, for every historical struct extern).
        widths = STRUCT_INIT_FIELD_WIDTHS[c_type]
        elem = KNOWN_STRUCT_SIZES[c_type]
        assert sum(widths) == elem, f'{c_type} STRUCT_INIT_FIELD_WIDTHS sums to {sum(widths)}, not {elem}'
        count = max(len(raw) // elem, 1)
        dims_suffix = f'[{count}]' if not dims else ''.join(f'[{d}]' for d in dims)
        records = []
        for i in range(count):
            rec, off = raw[i * elem:(i + 1) * elem], 0
            values = []
            for w in widths:
                values.append(str(int.from_bytes(rec[off:off + w], 'little')))
                off += w
            records.append('{ ' + ', '.join(values) + ' }')
        rows = ',\n    '.join(records)
        defn = f'{c_type} {ident}{dims_suffix} = {{\n    {rows}\n}};\n'
        return {'c_type': f'{c_type}{dims_suffix}', 'emit_kind': 'array', 'header_extra': [],
                'decl': f'extern {c_type} {ident}{dims_suffix};', 'defn': defn, 'verify_bytes': raw}

    if c_type in KNOWN_STRUCT_SIZES:
        # A field-by-field initializer would need this generic path to know
        # every game_structs.h struct's exact field layout (and, for
        # `struct dialog`/`gc0fe_record`/`gc0fe_catalog`, resolve embedded
        # pointer32 fields -- only available with a component's own `refs`,
        # which a plain symbol-driven DATA span does not carry). Safer to
        # keep the historical type in a comment and emit a byte-correct
        # `uint8_t` array than to guess a field layout that could silently
        # corrupt a pointer field.
        values = list(raw)
        n = len(values)
        defn = (f'/* historical type was {c_type} (see portable/include/game_structs.h); '
                f'no field-level initializer available outside a struct-shaped recipe '
                f'component, so the raw bytes are kept */\n'
                f'uint8_t {ident}[{n}] = {c_int_array(values)};\n')
        return {'c_type': f'uint8_t[{n}]', 'emit_kind': 'array', 'header_extra': [],
                'decl': f'extern uint8_t {ident}[{n}];', 'defn': defn, 'verify_bytes': raw,
                'untyped': True, 'untyped_note': f'historical type {c_type} not field-decomposed here'}

    # Plain dos_* scalar/array.
    elem_size = ELEM_SIZE_OF.get(c_type, 1)
    n_values = len(raw) // elem_size
    values = [int.from_bytes(raw[i * elem_size:(i + 1) * elem_size], 'little', signed=c_type in ('dos_char', 'dos_int', 'dos_long'))
              for i in range(n_values)]
    shape = dims or [n_values]
    if c_type == 'dos_char' and not dims and n_values > 1 and _is_c_string_like(raw):
        literal = c_escape_string(raw[:-1])
        defn = f'dos_char {ident}[{n_values}] = "{literal}";\n'
        return {'c_type': f'dos_char[{n_values}]', 'emit_kind': 'char_array', 'header_extra': [],
                'decl': f'extern dos_char {ident}[{n_values}];', 'defn': defn, 'verify_bytes': raw}
    dims_suffix = ''.join(f'[{d}]' for d in shape) if (dims or n_values != 1) else ''
    body = _nested_int_array(values, dims if len(dims) > 1 else [])
    if not dims_suffix:
        defn = f'{c_type} {ident} = {values[0]};\n'
        decl = f'extern {c_type} {ident};'
    else:
        defn = f'{c_type} {ident}{dims_suffix} = {body};\n'
        decl = f'extern {c_type} {ident}{dims_suffix};'
    return {'c_type': f'{c_type}{dims_suffix}', 'emit_kind': 'array', 'header_extra': [],
            'decl': decl, 'defn': defn, 'verify_bytes': raw}


TYPED_SCALAR_C = {'u8': 'dos_uchar', 'i8': 'dos_char', 'u16': 'dos_uint', 'i16': 'dos_int',
                  'u32': 'dos_ulong', 'i32': 'dos_long'}
TYPED_SCALAR_SIZE = {'u8': 1, 'i8': 1, 'u16': 2, 'i16': 2, 'u32': 4, 'i32': 4}
TYPED_SCALAR_STRUCT_FMT = {'u8': '<B', 'i8': '<b', 'u16': '<H', 'i16': '<h', 'u32': '<L', 'i32': '<l'}


# (offset, field name, C type, width, is_pointer32) -- include/DIALOG.H's
# struct dialog, byte-exact. Point 4: every typed-data-v1 component whose
# historical extern type is `struct dialog` and whose byte length is
# exactly 20 gets re-decomposed through this layout instead of an
# auto-generated anonymous struct (see _emit_as_known_struct).
DIALOG_FIELD_LAYOUT = [
    (0, 'kind', 'dos_int', 2, False),
    (2, 'title', 'dos_char', 4, True),
    (6, 'sub', 'dos_char', 1, False),
    (7, 'text', 'dos_char', 4, True),
    (11, 'initial', 'dos_uchar', 1, False),
    (12, 'cx', 'dos_int', 2, False),
    (14, 'cy', 'dos_int', 2, False),
    (16, 'w', 'dos_int', 2, False),
    (18, 'lines', 'dos_int', 2, False),
]


def _override_struct_field_width(fields):
    """Total byte width of ONE record built from a 'struct' override's
    `fields` list (farptr/nearptr/u8/i8/u16/i16/u32/i32/bytes:N)."""
    total = 0
    for _fname, kind in fields:
        if kind == 'bytes' or kind.startswith('bytes:'):
            total += int(kind.split(':', 1)[1]) if ':' in kind else 1
        else:
            total += OVERRIDE_FIELD_KIND[kind][0]
    return total


def _override_struct_record_parts(fields, tag, raw, refs_by_off, resolver):
    """One record's `{ .field = value, ... }` part list -- shared by both
    a scalar 'struct' override (count 1 or omitted) and an array of them
    (count > 1, e.g. menu_records_0CFA[3]); `raw`/`refs_by_off` are
    already sliced/rebased to THIS record's own [0, record_width)."""
    field_types = OVERRIDE_STRUCT_FIELD_C_TYPES.get(tag, {})
    parts, sub = [], 0
    for fname, kind in fields:
        if kind == 'bytes' or kind.startswith('bytes:'):
            n = int(kind.split(':', 1)[1]) if ':' in kind else 1
            parts.append(f'.{fname} = {c_int_array(list(raw[sub:sub + n]))}')
            sub += n
            continue
        width, is_ptr_field = OVERRIDE_FIELD_KIND[kind]
        if is_ptr_field:
            ref = refs_by_off.get(sub)
            if ref is not None:
                expr = resolver.expr(ref['target'], ref.get('addend', 0), _EMIT_KIND_REGISTRY)
                cast = field_types.get(fname)
                parts.append(f'.{fname} = {f"({cast})" if cast else ""}{expr}')
            elif all(b == 0 for b in raw[sub:sub + width]):
                parts.append(f'.{fname} = NULL')
            else:
                parts.append(f'.{fname} = NULL /* unresolved {kind} field, '
                              f'raw bytes {raw[sub:sub + width].hex()} -- needs a supervisor decision */')
        else:
            value = struct.unpack_from(TYPED_SCALAR_STRUCT_FMT[kind], raw, sub)[0]
            parts.append(f'.{fname} = {value}')
        sub += width
    return parts, sub


def _emit_override_struct(symbol, image, ident, resolver):
    """tools/portable/datagen_overrides.json 'struct' entry (top-level, or
    one entry of a 'layout' override -- see resolve_symbols): a designated
    initializer assembled from `struct.fields` (kind-tagged byte spans),
    with farptr/nearptr fields resolved through whatever recipe
    component's ref falls at that byte (collected into
    `symbol['override_refs']` by resolve_symbols, rebased to be relative
    to this override's own base offset) -- or a literal NULL when the raw
    bytes are all zero and nothing refs that offset. Scalar fields read
    straight off the DATA image. A resolved pointer is cast to
    OVERRIDE_STRUCT_FIELD_C_TYPES' declared field type when one is on
    file, matching the struct's real (game_funcs.h/game_structs.h-
    declared) field type.

    `symbol['override']['count']` (default 1), when > 1, repeats this same
    field layout that many times -- one record every `record_width` bytes
    -- and emits `struct TAG name[count] = { {...}, {...}, ... };` instead
    of a bare scalar (e.g. menu_records_0CFA[3]).
    """
    ov = symbol['override']
    tag = ov['struct']['tag']
    fields = ov['struct']['fields']
    count = ov.get('count', 1)
    record_width = _override_struct_field_width(fields)
    base = symbol['offset']
    raw = image[base:base + symbol['size']]
    refs_by_off = {r['offset']: r for r in symbol['override_refs']}
    assert record_width * count == symbol['size'], \
        f'override struct {tag!r}: {record_width} bytes/record * {count} != {symbol["size"]}'
    records = []
    for i in range(count):
        rec_raw = raw[i * record_width:(i + 1) * record_width]
        rec_refs = {off - i * record_width: r for off, r in refs_by_off.items()
                    if i * record_width <= off < (i + 1) * record_width}
        parts, consumed = _override_struct_record_parts(fields, tag, rec_raw, rec_refs, resolver)
        assert consumed == record_width, f'override struct {tag!r} fields sum to {consumed}, not {record_width}'
        records.append('{ ' + ', '.join(parts) + ' }')
    header_extra = []
    header_include = OVERRIDE_STRUCT_TAG_HEADER.get(tag)
    if header_include:
        header_extra.append(f'/* struct {tag} is declared in {header_include} */')
    if count > 1:
        rows = ',\n    '.join(records)
        defn = f'struct {tag} {ident}[{count}] = {{\n    {rows}\n}};\n'
        c_type, decl = f'struct {tag}[{count}]', f'extern struct {tag} {ident}[{count}];'
        emit_kind = 'record_array'
    else:
        defn = f'struct {tag} {ident} = {records[0]};\n'
        c_type, decl = f'struct {tag}', f'extern struct {tag} {ident};'
        emit_kind = 'struct'
    return {'c_type': c_type, 'emit_kind': emit_kind, 'header_extra': header_extra,
            'decl': decl, 'defn': defn, 'verify_bytes': raw}


def _emit_as_known_struct(component, image, struct_name, field_layout, ident, resolver, comp_by_id):
    """Decompose a struct-shaped component's raw bytes into a REAL
    portable/include/game_structs.h struct (point 4), resolving any
    pointer32 field through the component's own `refs` (falling back to a
    literal NULL when the field's raw bytes are all zero and no ref claims
    that offset -- e.g. DIALOG.H's "0 = none" title/text convention)."""
    ds0, length = component['ds_offset'], component['length']
    raw = image[ds0:ds0 + length]
    refs_by_off = {r['offset']: r for r in component['refs']}
    verify = bytearray(raw)
    init_parts = []
    for offset, name, c_type, width, is_pointer in field_layout:
        field_bytes = raw[offset:offset + width]
        if is_pointer:
            ref = refs_by_off.get(offset)
            if ref is not None:
                for i in range(offset, offset + width):
                    verify[i] = 0
                expr = resolver.expr(ref['target'], ref['addend'], _EMIT_KIND_REGISTRY)
                init_parts.append(f'.{name} = ({c_type} *)({expr})')
            elif all(b == 0 for b in field_bytes):
                init_parts.append(f'.{name} = 0')
            else:
                raise ValueError(f'{ident}.{name} (offset {offset}) is a non-zero pointer field '
                                  'with no ref to resolve it against')
        else:
            signed = c_type in ('dos_char', 'dos_int', 'dos_long')
            value = int.from_bytes(field_bytes, 'little', signed=signed)
            init_parts.append(f'.{name} = {value}')
    defn = f'{struct_name} {ident} = {{\n    ' + ',\n    '.join(init_parts) + '\n};\n'
    return {'c_type': struct_name, 'emit_kind': 'struct', 'header_extra': [],
            'decl': f'extern {struct_name} {ident};', 'defn': defn, 'verify_bytes': bytes(verify)}


def _emit_sound_ptr_array(symbol, resolver):
    """A run of unnamed pointer32/offset16 words inside the (now symbol-
    split) DATA_01139E_SOUND component -- e.g. the 36-entry dispatch table
    at DS:1832 or the 2-entry note-bank pointer pair at DS:182C. Each
    element resolves through the same PointerResolver every other
    component pointer field uses."""
    ident = c_ident(symbol['primary'])
    n = len(symbol['refs'])
    exprs = [resolver.expr(r['target'], r.get('addend', 0), _EMIT_KIND_REGISTRY) for r in symbol['refs']]
    body = ',\n    '.join(f'(void *)({e})' for e in exprs)
    defn = f'void *{ident}[{n}] = {{\n    {body}\n}};\n'
    return {'c_type': f'void *[{n}]', 'emit_kind': 'array', 'header_extra': [],
            'decl': f'extern void *{ident}[{n}];', 'defn': defn, 'verify_bytes': b'\x00' * symbol['size']}


def _interior_view_macros(component, table, friendly_of_offset, externs, base_expr,
                           container_local_base, include_gaps=True, gap_prefix='sound_instr_region',
                           own_names=frozenset(), container_struct_type=None, container_ident=None):
    """Split `component`'s own byte range into named pieces (and, when
    `include_gaps`, byte gaps too), like _split_sound_component but with no
    pointer refs to resolve, and return them as (name, macro_expr) pairs
    that VIEW into existing storage -- `base_expr` is a C expression for a
    byte pointer (`dos_char *`/`uint8_t *`-compatible) to the start of that
    storage, `container_local_base` bytes before this component's own
    start -- instead of creating new top-level objects. Used both for
    rule F's shared union storage and (round 5) for a struct-shaped
    component's OWN storage, so an interior name's byte-cast alias is
    always correct regardless of whether it lines up with that struct's
    own (arbitrarily-named) field boundaries.

    `own_names` are names ALREADY claimed by the real top-level object this
    view is attached to (its own primary + aliases, e.g. `gb2a`/
    `DATA_01075A_FILE_ERROR_CONTROL`) -- typically sitting at local offset
    0 with a fully-dimensioned historical type (e.g. `struct dialog`) that
    would otherwise swallow the *entire* span in one step and hide any
    OTHER interior name (e.g. `gb31`) from ever being reached; skipped
    byte-by-byte instead of measured, so the scan can keep going past them.

    `container_struct_type`/`container_ident`: when the container ITSELF
    is one of STRUCT_FIELD_LAYOUTS_FOR_ALIASING's known structs (e.g.
    `gb2a`: `struct dialog`), an interior POINTER-valued name that lands
    exactly on one of the container's OWN named fields (e.g. `gb31` on
    `.text`) becomes a `(container.field)` dot-notation macro instead of a
    raw `(*(T **)((dos_char *)&container + N))` byte-offset cast -- the
    portable struct's pointer fields are 8 bytes now, not the historical
    4/2, so a byte offset computed from the OLD layout silently reads the
    wrong bytes (a real crash class: bring-up hit exactly this reading
    `menu_list_draw`'s callbacks through a byte-offset macro). A field
    reference the compiler resolves by name is immune to that; a raw
    byte-offset one is used only when no such field mapping exists (the
    container is a plain byte blob, not a real known struct)."""
    comp_base, length, comp_id = component['ds_offset'], component['length'], component['id']

    def names_at(local):
        return table.names_at(comp_base + local) - {comp_id}

    macros = []
    local = 0
    while local < length:
        if names_at(local) & own_names:
            local += 1
            continue
        names = names_at(local)
        off = container_local_base + local
        if not names:
            start_abs = comp_base + local
            local += 1
            while local < length and not names_at(local):
                local += 1
            if include_gaps:
                name = f'{gap_prefix}_{start_abs:04X}'
                macros.append((name, f'((uint8_t *)({base_expr} + {off}))'))
            continue
        abs_off = comp_base + local
        primary = _pick_primary(names, table, friendly_of_offset, abs_off)
        chosen, chosen_name, _ = resolve_historical_type(names, externs, table, primary)
        c_type, dims, size, is_ptr = None, [], None, False
        if chosen is not None:
            mapped, _ = resolve_primitive(chosen['base'])
            c_type = mapped
            is_ptr = chosen['is_ptr'] or chosen['base'] == 'void (*)(void)'
            if chosen['base'] == 'void (*)(void)':
                c_type = 'void'
            dims = [v for v in (_c_int_literal(d) for d in chosen['dims']) if v is not None]
            size = _historical_span(chosen)
        if size is None:
            nxt = local + 1
            while nxt < length and not names_at(nxt):
                nxt += 1
            measured = nxt - local
            # An unspecified array (e.g. `int g1684[]`) still has a known
            # element width -- reshape the measured span with it instead of
            # discarding the type down to a raw byte cast.
            elem_size = (historical_elem_size(chosen['base'], chosen['is_ptr'],
                                               chosen.get('is_near', False), c_type)
                         if chosen is not None and c_type is not None else None)
            if elem_size and measured % elem_size == 0:
                dims = [measured // elem_size]
            else:
                c_type, dims = None, []
            size = measured
        field_path = (_struct_field_path(container_struct_type, local)
                      if is_ptr and container_struct_type and container_ident else None)
        if field_path is not None:
            # The container is a KNOWN struct and this interior name lands
            # exactly on one of its own named pointer fields: reference it
            # by NAME (`gb2a.text`), not by a historical byte offset --
            # the compiler resolves `.field` through the REAL (wide-
            # pointer) struct layout, so it is correct regardless of how
            # much the portable struct's own field offsets have shifted
            # from the historical ones.
            expr = f'({container_ident}{field_path})'
        elif c_type is None:
            expr = f'((uint8_t *)({base_expr} + {off}))'
        elif is_ptr:
            # The storage holds a POINTER VALUE (e.g. `char far *gb31`
            # landing on struct dialog's own `.text` field): cast to
            # "pointer to a `c_type *`" and dereference once, so the macro
            # itself evaluates to that pointer value, not to `*c_type`.
            expr = f'(*({c_type} **)({base_expr} + {off}))'
        elif dims:
            expr = f'(({c_type} *)({base_expr} + {off}))'
        else:
            expr = f'(*({c_type} *)({base_expr} + {off}))'
        # `names` is a set (from `names_at`/`table.names_at`); sort before
        # emitting so the order of same-offset macro lines in the generated
        # header (e.g. `g2fe4` vs `voice_byte_table`) is stable across
        # separate process invocations, not dependent on hash-randomized
        # set iteration order.
        for n in sorted(names):
            macros.append((n, expr))
        local += size
    return macros


def _emit_storage_union(image, symbol, union_info, comp_by_id, table, friendly_of_offset, externs):
    """Rule F: the raw-byte union object for a declared type that
    genuinely overlaps a neighboring struct-shaped component's storage.
    Emits the union as `uint8_t <name>[N]` from the DATA image, plus the
    struct typedef the overlapping component's own emitter would have
    produced (needed for the whole-struct macro cast, even though no
    separate instance of it exists any more), the overlapping declared
    type's own macro cast, AND (since the absorbed component is itself
    symbol-split, per the follow-up round) one macro view per historically
    named interior symbol / unnamed byte gap inside it."""
    offset, size = symbol['offset'], symbol['size']
    raw = image[offset:offset + size]
    orig_name, union_component_id = union_info['members']
    union_component = comp_by_id[union_component_id]
    ident = c_ident(union_info['union_name'])
    offset_into_union = union_component['ds_offset'] - offset

    struct_lines = []
    macros = []
    if union_component['format'] == INSTR_FORMAT:
        comp_ident = c_ident(union_component_id)
        struct_lines, _ = _sound_instruments_struct_lines(comp_ident)
        macros.append((union_component_id,
                        f'(*(struct {comp_ident}_s *)({ident} + {offset_into_union}))'))
        macros.extend(_interior_view_macros(union_component, table, friendly_of_offset, externs,
                                             ident, offset_into_union))
    else:
        raise ValueError(f'rule F union with {union_component["format"]!r} is not implemented; '
                          'needs a supervisor decision (see docs/portable/state-map.md)')
    # The overlapping declared type (ui_panel_glyph_records: struct
    # g2fd2_entry) is cast at offset 0 of the union.
    macros.append((orig_name, f'((struct g2fd2_entry *)({ident}))'))

    defn = f'uint8_t {ident}[{size}] = {c_int_array(list(raw))};\n'
    decl = f'extern uint8_t {ident}[{size}];'
    return {'c_type': f'uint8_t[{size}]', 'emit_kind': 'array', 'header_extra': struct_lines,
            'decl': decl, 'defn': defn, 'verify_bytes': raw, 'extra_macros': macros}


def _sound_instruments_struct_lines(ident):
    tag = f'{ident}_s'
    rec_tag = f'{ident}_record_s'
    struct_lines = [
        f'struct {rec_tag} {{ dos_int fields[28]; }};',
        f'struct {tag} {{',
        '    dos_uchar slot_glyph_pairs_tail[17];',
        '    dos_uchar voice_operator_offsets[18];',
        '    dos_uchar voice_disabled[18];',
        '    dos_uchar voice_connection[18];',
        f'    struct {rec_tag} instrument_records[33];',
        '    dos_int terminator_words[2];',
        '};',
    ]
    return struct_lines, tag


def _emit_typed_data(component, raw, ident, resolver, comp_by_id):
    fields = component['extra']['fields']
    refs_by_off = {r['offset']: r for r in component['refs']}
    tag = f'{ident}_s'
    struct_lines = [f'struct {tag} {{']
    init_parts = []
    verify = bytearray(raw)
    cursor = 0
    for field in fields:
        name = c_ident(field['name'])
        kind = field.get('type')
        if kind in ('pointer32', 'offset16'):
            width = 4 if kind == 'pointer32' else 2
            ref = refs_by_off[cursor]
            for i in range(cursor, cursor + width):
                verify[i] = 0
            target_comp = comp_by_id.get(ref['target'])
            target_fmt = target_comp['format'] if target_comp else None
            ptr_type = pointer_field_c_type(target_fmt)
            struct_lines.append(f'    {ptr_type}{"" if ptr_type.endswith("*") else " "}{name};')
            emitted_kinds = _EMIT_KIND_REGISTRY
            expr = resolver.expr(ref['target'], ref['addend'], emitted_kinds)
            init_parts.append(f'.{name} = ({ptr_type})({expr})')
            cursor += width
        else:
            values = field['values']
            ctype = TYPED_SCALAR_C[kind]
            size = TYPED_SCALAR_SIZE[kind]
            if len(values) == 1:
                struct_lines.append(f'    {ctype} {name};')
                init_parts.append(f'.{name} = {values[0]}')
            else:
                struct_lines.append(f'    {ctype} {name}[{len(values)}];')
                init_parts.append(f'.{name} = {c_int_array(values)}')
            cursor += size * len(values)
    struct_lines.append('};')
    defn = (f'struct {tag} {ident} = {{\n    ' + ',\n    '.join(init_parts) + '\n};\n')
    return {'c_type': f'struct {tag}', 'emit_kind': 'struct',
            'header_extra': struct_lines, 'decl': f'extern struct {tag} {ident};',
            'defn': defn, 'verify_bytes': bytes(verify)}


def _emit_pointer_records(component, raw, ident, resolver, comp_by_id):
    records = component['extra']['records']
    refs_by_off = {r['offset']: r for r in component['refs']}
    tag = f'{ident}_record_s'
    # Field layout is fixed: DS:offset word, far ptr A, byte A, far ptr B,
    # byte B, 8-byte tail -- see tools/pointer_records.py FORMAT docstring.
    # Determine each record's pointer C type from its own target's format.
    verify = bytearray(raw)
    rows = []
    RECORD_SIZE = 2 + 4 + 1 + 4 + 1 + 8
    ptr_types = set()
    for i, rec in enumerate(records):
        off = i * RECORD_SIZE
        fields = [f'{rec["word"]}']
        for suffix, sub_off in (('a', 2), ('b', 2 + 4 + 1)):
            ptr = rec[f'pointer_{suffix}']
            byteval = rec[f'byte_{suffix}']
            if ptr is None:
                fields.append('0')
                ptr_types.add('void *')
            else:
                ref = refs_by_off[off + sub_off]
                for k in range(off + sub_off, off + sub_off + 4):
                    verify[k] = 0
                target_comp = comp_by_id.get(ref['target'])
                ptype = pointer_field_c_type(target_comp['format'] if target_comp else None)
                ptr_types.add(ptype)
                expr = resolver.expr(ref['target'], ref['addend'], _EMIT_KIND_REGISTRY)
                fields.append(f'({ptype})({expr})')
            fields.append(str(byteval))
        fields.append('{' + ', '.join(str(b) for b in rec['tail']) + '}')
        rows.append('    {' + ', '.join(fields) + '}')
    ptr_ctype = ptr_types.pop() if len(ptr_types) == 1 else 'void *'
    struct_lines = [
        f'struct {tag} {{', '    dos_uint word;', f'    {ptr_ctype} pointer_a;',
        '    dos_uchar byte_a;', f'    {ptr_ctype} pointer_b;', '    dos_uchar byte_b;',
        '    dos_uchar tail[8];', '};',
    ]
    n = len(records)
    defn = f'struct {tag} {ident}[{n}] = {{\n' + ',\n'.join(rows) + '\n};\n'
    return {'c_type': f'struct {tag}[{n}]', 'emit_kind': 'record_array',
            'header_extra': struct_lines, 'decl': f'extern struct {tag} {ident}[{n}];',
            'defn': defn, 'verify_bytes': bytes(verify)}


def _emit_sound_data(component, raw, ident, resolver, comp_by_id):
    doc = component['extra']['doc']
    refs_by_off = {r['offset']: r for r in component['refs']}
    tag = f'{ident}_s'
    struct_lines = [f'struct {tag} {{']
    init_parts = []
    verify = bytearray(raw)
    cursor = 0

    def words(key):
        nonlocal cursor
        values = doc[key]
        struct_lines.append(f'    dos_uint {key}[{len(values)}];')
        init_parts.append(f'.{key} = {c_int_array(values)}')
        cursor += 2 * len(values)

    words('state_words')
    words('note_divisors')
    for bank_name in doc['note_banks']:
        ref = refs_by_off[cursor]
        for k in range(cursor, cursor + 2):
            verify[k] = 0
        expr = resolver.expr(ref['target'], 0, _EMIT_KIND_REGISTRY)
        fname = c_ident(bank_name) + '_ptr'
        struct_lines.append(f'    dos_uint *{fname};')
        init_parts.append(f'.{fname} = (dos_uint *)({expr})')
        cursor += 2
    struct_lines.append('    dos_uint opl_port;')
    init_parts.append(f".opl_port = {doc['opl_port']}")
    cursor += 2
    dispatch = doc['dispatch']
    struct_lines.append(f'    dos_uchar *dispatch[{len(dispatch)}];')
    dispatch_init = []
    for target in dispatch:
        ref = refs_by_off[cursor]
        for k in range(cursor, cursor + 2):
            verify[k] = 0
        expr = resolver.expr(ref['target'], 0, _EMIT_KIND_REGISTRY)
        dispatch_init.append(f'(dos_uchar *)({expr})')
        cursor += 2
    init_parts.append('.dispatch = {\n        ' + ',\n        '.join(dispatch_init) + '\n    }')
    prefix = doc['lookup_prefix']
    struct_lines.append(f'    dos_uchar lookup_prefix[{len(prefix)}];')
    init_parts.append(f'.lookup_prefix = {c_int_array(prefix)}')
    cursor += len(prefix)
    for table_doc in doc['lookup_tables']:
        name = c_ident(table_doc['id'])
        values = table_doc['values']
        struct_lines.append(f'    dos_uchar {name}[{len(values)}];')
        init_parts.append(f'.{name} = {c_int_array(values)}')
        cursor += len(values)
    tail = doc['tail_words']
    struct_lines.append(f'    dos_uint tail_words[{len(tail)}];')
    init_parts.append(f'.tail_words = {c_int_array(tail)}')
    cursor += 2 * len(tail)
    struct_lines.append('};')
    if cursor != len(raw):
        raise ValueError(f'sound data layout consumed {cursor} of {len(raw)} bytes')
    defn = f'struct {tag} {ident} = {{\n    ' + ',\n    '.join(init_parts) + '\n};\n'
    return {'c_type': f'struct {tag}', 'emit_kind': 'struct',
            'header_extra': struct_lines, 'decl': f'extern struct {tag} {ident};',
            'defn': defn, 'verify_bytes': bytes(verify)}


def _emit_sound_instruments(component, raw, ident):
    doc = component['extra']['doc']
    struct_lines, tag = _sound_instruments_struct_lines(ident)
    records_init = ',\n        '.join(
        '{' + c_int_array(r) + '}' for r in doc['instrument_records'])
    init_parts = [
        f".slot_glyph_pairs_tail = {c_int_array(doc['slot_glyph_pairs_tail'])}",
        f".voice_operator_offsets = {c_int_array(doc['voice_operator_offsets'])}",
        f".voice_disabled = {c_int_array(doc['voice_disabled'])}",
        f".voice_connection = {c_int_array(doc['voice_connection'])}",
        '.instrument_records = {\n        ' + records_init + '\n    }',
        f".terminator_words = {c_int_array(doc['terminator_words'])}",
    ]
    defn = f'struct {tag} {ident} = {{\n    ' + ',\n    '.join(init_parts) + '\n};\n'
    return {'c_type': f'struct {tag}', 'emit_kind': 'struct',
            'header_extra': struct_lines, 'decl': f'extern struct {tag} {ident};',
            'defn': defn, 'verify_bytes': raw}


# Populated during emit_game_data(); PointerResolver.expr() consults it to
# decide between `name` (array decay) and `&name` for a same-object pointer.
_EMIT_KIND_REGISTRY = {}

# primary_name -> {local_byte_offset: field_c_ident}, populated for every
# typed-data-v1 component before pointer resolution runs (a typed-data
# pointer field may target a specific field of *another* typed-data struct,
# e.g. DATA_010924_MENU_DESCRIPTORS' own descriptor_pointer_006 fields chain
# to each other's non-zero field offsets).
_STRUCT_FIELD_REGISTRY = {}


def _sound_data_field_layout(doc):
    """Mirrors _emit_sound_data's cursor bookkeeping so a pointer that lands
    on a *named field* of the sound-data struct (not just its base) --
    e.g. the note_divisors/note_divisors_octave banks (the latter indexes
    halfway into the former, per tools/sound_data.py's bank_size math) or a
    lookup_XXXX table -- resolves to `&sound_struct.field[i]` instead of
    failing.  Returns a list of (start, field_name, element_size, length).
    """
    layout = []
    cursor = 0

    def mark(name, elem_size, count):
        nonlocal cursor
        length = elem_size * count
        layout.append((cursor, name, elem_size, length))
        cursor += length

    mark('state_words', 2, len(doc['state_words']))
    mark('note_divisors', 2, len(doc['note_divisors']))
    for bank_name in doc['note_banks']:
        mark(c_ident(bank_name) + '_ptr', 2, 1)
    mark('opl_port', 2, 1)
    mark('dispatch', 2, len(doc['dispatch']))
    mark('lookup_prefix', 1, len(doc['lookup_prefix']))
    for t in doc['lookup_tables']:
        mark(c_ident(t['id']), 1, len(t['values']))
    mark('tail_words', 2, len(doc['tail_words']))
    return layout


def _typed_data_field_layout(fields):
    layout = []
    cursor = 0
    for field in fields:
        name = c_ident(field['name'])
        kind = field.get('type')
        if kind == 'pointer32':
            layout.append((cursor, name, 4, 4))
            cursor += 4
        elif kind == 'offset16':
            layout.append((cursor, name, 2, 2))
            cursor += 2
        else:
            size = TYPED_SCALAR_SIZE[kind]
            count = len(field['values'])
            layout.append((cursor, name, size, size * count))
            cursor += size * count
    return layout

_FORMAT_EMIT_KIND = {
    'ascii-nul-v1': 'char_array', 'ascii-v1': 'char_array',
    'u16le-table-v1': 'array', 'zero-pad-v1': 'array', 'fixed-records-v1': 'array',
    'dac6-rgb256-v1': 'array', TYPED_FORMAT: 'struct', PTRREC_FORMAT: 'record_array',
    SOUND_FORMAT: 'struct', INSTR_FORMAT: 'struct',
}

GENERATED_DIR = ROOT / 'portable/generated'
DOCS_DIR = ROOT / 'docs/portable'

HEADER_BANNER = """/* {name} -- GENERATED by tools/portable/datagen.py. DO NOT EDIT BY HAND.
 * Regenerate with: python tools/portable/datagen.py
 * See portable/generated/README.md and docs/portable/state-map.md.
 */
"""


def emit_game_data(image, components, symbols, table, externs, extra, qualifiers, friendly_of_offset, out_dir):
    """Objects are symbol-driven (see resolve_symbols): each generated DATA
    symbol's `emit_path` says which emitter produces it --
    'struct-component' (describe_and_emit_component's TYPED/SOUND/PTRREC/
    INSTR branches -- a whole struct-shaped recipe component with real
    pointer resolution), 'component-fallback' (describe_and_emit_component's
    flat-format branches -- a whole anonymous component, no better name
    exists for any offset inside it), or 'flat' (emit_generic_flat_object --
    a plain symbol-driven span typed from the historical extern/name
    inventory, unrelated to component boundaries).

    Per point 2's "PRIMARY NAME" rule, a component id becomes a `#define`
    alias only when something actually points at it (a pointer32/offset16
    ref target); otherwise it is left as a comment so unreferenced
    synthetic ids do not pollute the namespace.
    """
    _EMIT_KIND_REGISTRY.clear()
    _STRUCT_FIELD_REGISTRY.clear()
    comp_by_id = {c['id']: c for c in components}
    data_symbols = sorted((s for s in symbols if s['section'] == 'data'), key=lambda s: s['offset'])
    referenced_component_ids = {ref['target'] for c in components for ref in c.get('refs', [])}
    blocked_field_names = scan_struct_field_names()

    # Pass 1: seed the pointer-target registry (needed before Pass 2
    # resolves any &target/target expression, since a component earlier in
    # file order may point at one defined later). Every generated DATA
    # object decays like an array for &-purposes except the rare
    # unresolved-struct/non-NULL-pointer flat fallback, which is a plain
    # uint8_t[] anyway (still array-shaped).
    for s in data_symbols:
        if s['definition_site'] != 'generated':
            continue
        if s['emit_path'] == 'struct-component':
            c = comp_by_id[s['component_id']]
            _EMIT_KIND_REGISTRY[s['primary']] = _FORMAT_EMIT_KIND[c['format']]
            is_dialog = (c['format'] == TYPED_FORMAT and c['length'] == KNOWN_STRUCT_SIZES['struct dialog']
                         and any(d['base'] == 'struct dialog' for d in externs.get(s['primary'], ())))
            if is_dialog:
                _STRUCT_FIELD_REGISTRY[s['primary']] = [
                    (off, name, width, width) for off, name, _, width, _ in DIALOG_FIELD_LAYOUT]
            elif c['format'] == TYPED_FORMAT:
                _STRUCT_FIELD_REGISTRY[s['primary']] = _typed_data_field_layout(c['extra']['fields'])
        elif s['emit_path'] == 'ptrrec-dialog':
            _EMIT_KIND_REGISTRY[s['primary']] = 'struct'
            _STRUCT_FIELD_REGISTRY[s['primary']] = [
                (off, name, width, width) for off, name, _, width, _ in DIALOG_FIELD_LAYOUT]
        elif s['emit_path'] == 'override-struct':
            if s['override'].get('count', 1) > 1:
                # An ARRAY of the struct (e.g. menu_records_0CFA[3]) is
                # only ever pointed at whole (array decay to its first
                # element) by everything currently on file -- no per-
                # element field registry needed for that.
                _EMIT_KIND_REGISTRY[s['primary']] = 'record_array'
            else:
                _EMIT_KIND_REGISTRY[s['primary']] = 'struct'
                fields, off = [], 0
                for fname, kind in s['override']['struct']['fields']:
                    width = OVERRIDE_FIELD_KIND[kind][0]
                    fields.append((off, fname, width, width))
                    off += width
                _STRUCT_FIELD_REGISTRY[s['primary']] = fields
        else:
            # A plain scalar (no dims, not the untyped-fallback byte array)
            # needs `&name`, not array-decay, if something ever points at
            # it; everything else (arrays, uint8_t[] fallbacks, the sound
            # ptr-array objects) decays like a normal C array.
            is_scalar = not s['dims'] and not s['untyped'] and s['emit_path'] != 'sound-ptr-array' \
                and s['c_type'] not in (None,) and s['c_type'] != 'void (*)(void)'
            _EMIT_KIND_REGISTRY[s['primary']] = 'scalar' if is_scalar else 'array'

    # A DATA pointer (an override-struct field, or any other symbolic ref)
    # can legitimately target a BSS object -- emit_game_state() hasn't run
    # yet (game_data.[ch] is built first), so without this, resolving such
    # a ref would find no emit_kind for the BSS target and default to
    # `&name` even when the target is really an array that should decay
    # (e.g. `gc5ce`: `dos_char *gc5ce[3]`, target of g22f0's `.records`
    # field -- this fell back to `&gc5ce` instead of `gc5ce` before this
    # loop existed). Same is_scalar heuristic emit_game_state() itself
    # effectively uses (c_type set, no array dims -> scalar).
    for s in symbols:
        if s['section'] != 'bss' or s['definition_site'] != 'generated':
            continue
        is_scalar = not s['dims'] and s['c_type'] not in (None, 'jmp_buf') and s['c_type'] != 'void (*)(void)'
        _EMIT_KIND_REGISTRY[s['primary']] = 'scalar' if is_scalar else 'array'

    resolver = PointerResolver(table, symbols)

    # A DATA pointer can legitimately target a BSS object (e.g. an initial
    # pointer into a runtime buffer); game_data.h then needs game_state.h's
    # extern declarations visible.  Checked up front so the #include is
    # emitted before any use, regardless of component processing order.
    needs_game_state_h = False
    for c in components:
        for ref in c.get('refs', ()):
            sym, _ = resolver.resolve(ref['target'], ref.get('addend', 0))
            if sym['section'] == 'bss':
                needs_game_state_h = True
                break
        if needs_game_state_h:
            break

    # A 'struct' override whose tag is not one of game_structs.h's
    # KNOWN_STRUCT_SIZES (e.g. `struct input`, DIALOG.C-local, declared in
    # game_funcs.h) needs that header visible before its `extern struct
    # input g22f0;` declaration.
    needs_game_funcs_h = any(
        s['emit_path'] == 'override-struct'
        and s['override']['struct']['tag'] in OVERRIDE_STRUCT_TAG_HEADER
        for s in data_symbols)

    header_lines = [HEADER_BANNER.format(name='game_data.h'), '#ifndef PORTABLE_GAME_DATA_H',
                     '#define PORTABLE_GAME_DATA_H', '', '#include <stdint.h>',
                     '#include "dos_types.h"', '#include "game_structs.h"']
    if needs_game_funcs_h:
        header_lines.append('#include "game_funcs.h"  /* struct input (DIALOG.C-local) */')
    if needs_game_state_h:
        header_lines.append('#include "game_state.h"  /* a DATA pointer targets BSS state */')
    header_lines.append('')
    # A code-pointer table (point 2) casts each element to a NAMED ported
    # function (e.g. `(void (*)(void))roundend_draw_marker`) -- those
    # prototypes are declared in game_funcs.h, needed only in the .c file
    # (the .h declaration itself, `extern void (*g12a1[6])(void);`, names
    # no other type).
    needs_game_funcs_h_source = any(
        s['c_type'] == 'void (*)(void)' and s['dims'] and s['definition_site'] == 'generated'
        for s in data_symbols)
    source_lines = [HEADER_BANNER.format(name='game_data.c'), '#include "game_data.h"']
    if needs_game_funcs_h_source:
        source_lines.append('#include "game_funcs.h"  /* code-pointer table function names */')
    source_lines.append('')

    verify_bytes = {}
    emitted_count = 0
    skipped_owned = 0

    # Point 5, "ported-C-owned DATA": the ported .c file IS the owner, full
    # stop -- this must NOT depend on the generator already having offset
    # knowledge for the name (most of PROMPTS.C's own block never got one:
    # energy_meter/hud_prompt_kind/gb85 have no symbols.json entry at all,
    # per portable/game/hud.c's own blocking comment). So: every top-level
    # object DEFINITION scan_ported_c_definitions() finds in
    # portable/game/*.c (never a `static` one -- the regex only matches a
    # line starting with a bare dos_* type, which a `static` prefix can
    # never do -- and never a function, which needs an argument list
    # between the name and `;`/`{` that the regex has no room for) gets an
    # `extern` declaration in game_data.h -- UNLESS the generator already
    # emits a 'generated' object under that same name (primary or alias)
    # elsewhere, which would collide. When an offset IS separately known
    # (a 'ported-C:*' symbol's own name/alias), the DS offset and
    # component id are shown too, for the state-map.md row; otherwise the
    # declaration still goes out, just without that provenance.
    ported_defs = scan_ported_c_definitions()
    generated_names = {n for s in symbols if s['definition_site'] == 'generated'
                        for n in (s['primary'], *s['aliases'])}
    offset_of_ported_name = {}
    for s in data_symbols:
        if not s['definition_site'].startswith('ported-C:'):
            continue
        for name in s['names']:
            offset_of_ported_name[name] = (s['offset'], s['component_id'])
    ported_c_owned_report = []
    for name in sorted(ported_defs):
        if name in generated_names:
            continue
        file_path, decl = ported_defs[name]
        offset, component_id = offset_of_ported_name.get(name, (None, None))
        loc = f"DS:{offset:04X}, component {component_id}" if offset is not None \
            else 'DS offset not known to any of the 7 symbol sources'
        header_lines.append(f'/* {name}  ({loc}) */')
        header_lines.append(f'extern {decl};  /* defined in {file_path} */')
        header_lines.append('')
        ported_c_owned_report.append({'name': name, 'offset': offset, 'file': file_path,
                                       'component_id': component_id})

    for s in data_symbols:
        if s['definition_site'] == 'subsystem-owned':
            skipped_owned += 1
            continue
        if s['definition_site'] != 'generated':
            continue  # ported-C:* (compiled-data): the ported .C file supplies this

        primary = c_ident(s['primary'])
        if s['emit_path'] == 'struct-component':
            c = comp_by_id[s['component_id']]
            dialog_decls = [d for d in externs.get(s['primary'], ()) if d['base'] == 'struct dialog']
            if c['format'] == TYPED_FORMAT and c['length'] == KNOWN_STRUCT_SIZES['struct dialog'] and dialog_decls:
                # Point 4: "struct dialog gb2a etc.: use the real struct
                # types" -- this typed-data component's own historical
                # extern type is `struct dialog` (20 bytes: kind/title/sub/
                # text/initial/cx/cy/w/lines) and its byte length matches
                # exactly, so re-decompose it into that real struct instead
                # of an auto-generated anonymous one.
                result = _emit_as_known_struct(c, image, 'struct dialog', DIALOG_FIELD_LAYOUT,
                                                primary, resolver, comp_by_id)
            else:
                result = describe_and_emit_component(c, image, table, resolver, comp_by_id, primary)
        elif s['emit_path'] == 'component-fallback':
            c = comp_by_id[s['component_id']]
            result = describe_and_emit_component(c, image, table, resolver, comp_by_id, primary)
        elif s['emit_path'] == 'storage-union':
            union_info = next(u for u in extra['storage_alias'] if u['union_name'] == s['primary'])
            result = _emit_storage_union(image, s, union_info, comp_by_id, table, friendly_of_offset,
                                          externs)
        elif s['emit_path'] == 'sound-ptr-array':
            result = _emit_sound_ptr_array(s, resolver)
        elif s['emit_path'] == 'ptrrec-dialog':
            c = comp_by_id[s['component_id']]
            rec_start = s['offset'] - c['ds_offset']
            record = {
                'ds_offset': s['offset'], 'length': s['size'],
                'refs': [{**r, 'offset': r['offset'] - rec_start} for r in c['refs']
                         if rec_start <= r['offset'] < rec_start + s['size']],
            }
            result = _emit_as_known_struct(record, image, 'struct dialog', DIALOG_FIELD_LAYOUT,
                                            primary, resolver, comp_by_id)
        elif s['emit_path'] == 'override-struct':
            result = _emit_override_struct(s, image, primary, resolver)
        else:
            result = emit_generic_flat_object(s, image, primary)

        tag = f"{s['emit_path']}, component {s['component_id']}" if s['component_id'] else s['emit_path']
        header_lines.append(f"/* {s['primary']}  DS:{s['offset']:04X}  size {s['size']}  ({tag}) */")
        header_lines += result['header_extra']
        qualifier = qualifiers.get(s['primary'])
        decl, defn = result['decl'], result['defn']
        if qualifier:
            decl, defn = _qualify_decl(decl, qualifier), _qualify_defn(defn, qualifier)
        header_lines.append(decl)
        for alias in s['aliases']:
            if alias == s['component_id'] and alias not in referenced_component_ids:
                header_lines.append(f'/* {alias} (recipe component id; nothing points at it) */')
                continue
            reason = alias_macro_block_reason(alias, blocked_field_names)
            if reason:
                extra['short_aliases'].append({'name': alias, 'target': primary, 'reason': reason})
                header_lines.append(f'/* {alias} -> {primary} ({reason}; no macro -- use {primary} '
                                     f'directly. See state-map.md "Short aliases") */')
            else:
                header_lines.append(f'#define {c_ident(alias)} {primary}')
        for macro_name, macro_expr in result.get('extra_macros', []):
            reason = alias_macro_block_reason(macro_name, blocked_field_names)
            if reason:
                extra['short_aliases'].append({'name': macro_name, 'target': macro_expr, 'reason': reason})
                header_lines.append(f'/* {macro_name} -> {macro_expr} ({reason}; no macro. See '
                                     f'state-map.md "Short aliases") */')
            else:
                header_lines.append(f'#define {c_ident(macro_name)} {macro_expr}')
        header_lines.append('')
        source_lines.append(defn)
        verify_bytes[s['primary']] = result['verify_bytes']
        # Feed the emitted type back into the symbol record so
        # symbols.json/state-map.md report the real C type instead of
        # showing every DATA object as "untyped".
        s['c_type'] = result['c_type']
        s['untyped'] = bool(result.get('untyped'))
        if result.get('untyped'):
            s['type_note'] = (s['type_note'] or '') + (result.get('untyped_note') or '')
        emitted_count += 1

    skipped_code_owned = sum(1 for c in components if c['kind'] == 'code_owned')
    skipped_toolchain = sum(1 for c in components if c['kind'] == 'toolchain_opaque')

    # Round 5: typed-data-v1 struct-shaped components with a historically
    # named interior symbol (e.g. gb31 landing on DATA_01075A_FILE_ERROR_
    # CONTROL's own 'text' pointer field) get one alias macro per name,
    # computed the same byte-offset-cast way as the round-4 sound-
    # instruments interior split -- correct regardless of whether the name
    # happens to line up with the component's own (arbitrarily-generated)
    # field boundaries. No gap markers here: the struct's own fields
    # already fully and correctly cover every byte (unlike the round-4
    # union case, which had genuinely unaccounted-for storage).
    for s in data_symbols:
        if s['emit_path'] != 'struct-component' or s['definition_site'] != 'generated':
            continue
        c = comp_by_id[s['component_id']]
        if c['format'] != TYPED_FORMAT:
            continue
        primary_ident = c_ident(s['primary'])
        base_expr = f'((dos_char *)(&{primary_ident}))'
        own_names = {s['primary'], *s['aliases']}
        container_struct_type = s['c_type'] if s['c_type'] in STRUCT_FIELD_LAYOUTS_FOR_ALIASING else None
        for name, expr in _interior_view_macros(c, table, friendly_of_offset, externs, base_expr, 0,
                                                  include_gaps=False, own_names=own_names,
                                                  container_struct_type=container_struct_type,
                                                  container_ident=primary_ident):
            extra['interior_alias'].append({
                'name': name, 'offset': table.offset_by_name.get(name, c['ds_offset']),
                'array_name': s['primary'], 'array_offset': c['ds_offset'],
                'expr': expr, 'c_type': s['c_type'],
            })

    data_interior = [ia for ia in extra['interior_alias'] if ia['offset'] < DATA_LEN]
    if data_interior:
        header_lines.append('/* Rule B: interior alias expressions (see docs/portable/state-map.md '
                             '"Interior aliases") */')
        for ia in data_interior:
            reason = alias_macro_block_reason(ia['name'], blocked_field_names)
            if reason:
                extra['short_aliases'].append({'name': ia['name'], 'target': ia['expr'], 'reason': reason})
                header_lines.append(f"/* {ia['name']} -> {ia['expr']} ({reason}; no macro. See "
                                     f'state-map.md "Short aliases") */')
            else:
                header_lines.append(f"#define {c_ident(ia['name'])} {ia['expr']}")
        header_lines.append('')

    header_lines.append('#endif /* PORTABLE_GAME_DATA_H */')
    header_lines.append('')

    out_dir.mkdir(parents=True, exist_ok=True)
    (out_dir / 'game_data.h').write_text('\n'.join(header_lines), encoding='utf-8', newline='\n')
    (out_dir / 'game_data.c').write_text('\n'.join(source_lines), encoding='utf-8', newline='\n')

    return {
        'emitted': emitted_count, 'skipped_owned': skipped_owned,
        'skipped_code_owned': skipped_code_owned, 'skipped_toolchain': skipped_toolchain,
        'verify_bytes': verify_bytes, 'ported_c_owned': ported_c_owned_report,
    }


def emit_game_state(symbols, extra, qualifiers, out_dir):
    """portable/generated/game_state.[ch]: one zero-initialized BSS object
    per unowned BSS symbol, typed from layout/production-plan.json['bss']
    typed_reserves when known, from an extern `/* DS:XXXX */` comment when
    typed_reserves has nothing, else `uint8_t name[N]` (task's fallback
    rule) -- static storage is zero-initialized either way, so none of these
    need an explicit initializer.
    """
    header_lines = [HEADER_BANNER.format(name='game_state.h'), '#ifndef PORTABLE_GAME_STATE_H',
                     '#define PORTABLE_GAME_STATE_H', '', '#include <stdint.h>',
                     '#include "dos_types.h"', '#include "game_structs.h"', '']
    source_lines = [HEADER_BANNER.format(name='game_state.c'), '#include "game_state.h"', '']

    bss_symbols = [s for s in symbols if s['section'] == 'bss']
    emitted = 0
    skipped_owned = 0
    blocked_field_names = scan_struct_field_names()
    typed_count = 0
    untyped_count = 0
    needs_setjmp = any(s['section'] == 'bss' and s['definition_site'] == 'generated'
                        and s['c_type'] == 'jmp_buf' for s in symbols)
    if needs_setjmp:
        header_lines.insert(6, '#include <setjmp.h>  /* rule G: jmp_buf game_abort_jmpbuf */')

    elem_size_of = {'dos_char': 1, 'dos_uchar': 1, 'dos_int': 2, 'dos_uint': 2,
                    'dos_long': 4, 'dos_ulong': 4, 'uint8_t': 1, **KNOWN_STRUCT_SIZES}

    for s in bss_symbols:
        if s['definition_site'] != 'generated':
            skipped_owned += 1
            continue
        name = c_ident(s['primary'])
        if s['c_type']:
            base = s['c_type']
            typed_count += 1
        else:
            base = 'uint8_t'
            untyped_count += 1

        if base == 'jmp_buf':
            # Rule G: jmp_buf is already an array type in C -- one host
            # jmp_buf object, never `jmp_buf name[N]`.
            header_lines.append(f"/* {s['primary']}  DS:{s['offset']:04X}  size {s['size']} "
                                 '(rule G: host jmp_buf, historical size was Turbo C\'s 20 bytes) */')
            qualifier = qualifiers.get(s['primary'])
            jb_decl = f'extern jmp_buf {name};'
            jb_defn = f'jmp_buf {name};\n'
            if qualifier:
                jb_decl, jb_defn = _qualify_decl(jb_decl, qualifier), _qualify_defn(jb_defn, qualifier)
            header_lines.append(jb_decl)
            for alias in s['aliases']:
                reason = alias_macro_block_reason(alias, blocked_field_names)
                if reason:
                    extra['short_aliases'].append({'name': alias, 'target': name, 'reason': reason})
                    header_lines.append(f'/* {alias} -> {name} ({reason}; no macro. See '
                                         f'state-map.md "Short aliases") */')
                else:
                    header_lines.append(f'#define {c_ident(alias)} {name}')
            header_lines.append('')
            source_lines.append(jb_defn)
            emitted += 1
            continue

        # `dims` describes the shape of the *resolved* base type (e.g. the
        # struct-element count in 'struct record3e8[10]'); once the base
        # falls back to uint8_t because that struct has no portable
        # definition, the dims no longer denote bytes, so they must not be
        # reused -- fall through to the byte-span sizing below instead.
        has_shaped_dims = bool(s['dims']) and s['c_type'] is not None
        if s['is_ptr']:
            dims_suffix = ''.join(f'[{d}]' for d in s['dims'])
            decl = f'extern {base} *{name}{dims_suffix};'
            defn = f'{base} *{name}{dims_suffix};\n'
            display_type = f'{base} *{dims_suffix}'
        elif has_shaped_dims:
            dims_suffix = ''.join(f'[{d}]' for d in s['dims'])
            decl = f'extern {base} {name}{dims_suffix};'
            defn = f'{base} {name}{dims_suffix};\n'
            display_type = f'{base}{dims_suffix}'
        else:
            # No declared shape: derive one from the measured byte span so a
            # multi-element object never collapses into a single scalar.
            elem = elem_size_of.get(base, 1)
            count = s['size'] // elem if elem and s['size'] % elem == 0 else None
            if count is None or count <= 1:
                decl = f'extern {base} {name};'
                defn = f'{base} {name};\n'
                display_type = base
            else:
                decl = f'extern {base} {name}[{count}];'
                defn = f'{base} {name}[{count}];\n'
                display_type = f'{base}[{count}]'
        s['c_type'] = display_type
        note = f" -- {s['type_note']}" if s.get('type_note') else ''
        header_lines.append(f"/* {s['primary']}  DS:{s['offset']:04X}  size {s['size']}{note} */")
        qualifier = qualifiers.get(s['primary'])
        if qualifier:
            decl, defn = _qualify_decl(decl, qualifier), _qualify_defn(defn, qualifier)
        header_lines.append(decl)
        for alias in s['aliases']:
            reason = alias_macro_block_reason(alias, blocked_field_names)
            if reason:
                extra['short_aliases'].append({'name': alias, 'target': name, 'reason': reason})
                header_lines.append(f'/* {alias} -> {name} ({reason}; no macro. See '
                                     f'state-map.md "Short aliases") */')
            else:
                header_lines.append(f'#define {c_ident(alias)} {name}')
        header_lines.append('')
        source_lines.append(defn)
        emitted += 1

    bss_interior = [ia for ia in extra['interior_alias'] if ia['offset'] >= DATA_LEN]
    if bss_interior:
        header_lines.append('/* Rule B: interior alias expressions (see docs/portable/state-map.md '
                             '"Interior aliases") */')
        for ia in bss_interior:
            reason = alias_macro_block_reason(ia['name'], blocked_field_names)
            if reason:
                extra['short_aliases'].append({'name': ia['name'], 'target': ia['expr'], 'reason': reason})
                header_lines.append(f"/* {ia['name']} -> {ia['expr']} ({reason}; no macro. See "
                                     f'state-map.md "Short aliases") */')
            else:
                header_lines.append(f"#define {c_ident(ia['name'])} {ia['expr']}")
        header_lines.append('')

    header_lines.append('#endif /* PORTABLE_GAME_STATE_H */')
    header_lines.append('')

    out_dir.mkdir(parents=True, exist_ok=True)
    (out_dir / 'game_state.h').write_text('\n'.join(header_lines), encoding='utf-8', newline='\n')
    (out_dir / 'game_state.c').write_text('\n'.join(source_lines), encoding='utf-8', newline='\n')

    return {'emitted': emitted, 'skipped_owned': skipped_owned,
            'typed': typed_count, 'untyped': untyped_count}


def write_symbols_json(components, symbols, table, notes, out_path):
    by_primary = {}
    for s in symbols:
        by_primary[s['primary']] = {
            'ds_offset': s['offset'], 'section': s['section'], 'size': s['size'],
            'c_type': s['c_type'], 'aliases': s['aliases'],
            'definition_site': s['definition_site'], 'owned_by': s['owned_by'],
            'component_id': s['component_id'], 'conflict': s['conflict'],
            'type_note': s['type_note'], 'untyped': s['untyped'],
            'alias_type_reports': s['alias_type_reports'],
        }
    alias_index = {}
    for s in symbols:
        for a in s['aliases']:
            alias_index[a] = s['primary']

    gaps = [{'id': g['id'], 'ds_offset': g['ds_offset'], 'length': g['length'],
             'classification': g['classification']} for g in notes['gaps']]
    code_owned = [{'id': c['id'], 'ds_offset': c['ds_offset'], 'length': c['length'],
                   'code_owner': c['code_owner']} for c in components if c['kind'] == 'code_owned']

    doc = {
        'format': 'portable-dgroup-symbols-v1',
        'data_len': DATA_LEN, 'bss_len': BSS_LEN, 'dgroup_len': DGROUP_LEN,
        'symbols': by_primary, 'aliases': alias_index,
        'toolchain_opaque_regions': gaps, 'code_owned_regions': code_owned,
    }
    out_path.write_text(json.dumps(doc, indent=2, sort_keys=True), encoding='utf-8', newline='\n')
    return doc


def write_state_map_md(components, symbols, table, notes, warnings, data_report, state_report,
                        exe_check, extra, out_path):
    lines = [
        '# Portable DGROUP state map',
        '',
        '_Generated by `tools/portable/datagen.py`; do not edit by hand. '
        'Regenerate with `python tools/portable/datagen.py`._',
        '',
        f'DATA image: {DATA_LEN} bytes (DS:0000..DS:{DATA_LEN:04X}), built from the '
        '`recipes/data/game-initialized.json` JSON sources (PRIMARY byte source -- see '
        '"EXE cross-check" below). '
        f'BSS: {BSS_LEN} bytes (DS:{DATA_LEN:04X}..DS:{DGROUP_LEN:04X}), from '
        '`src/data/GAME_BSS.json`.',
        '',
        '## Summary',
        '',
        f'- DATA components: {sum(1 for c in components if c["kind"] == "data")} carry recipe '
        f'bytes, {sum(1 for c in components if c["kind"] == "code_owned")} owned by a ported C '
        'file (their static initializers, not this generator, supply the bytes), '
        f'{sum(1 for c in components if c["kind"] == "toolchain_opaque")} toolchain/alignment '
        'bytes not modeled at all (Turbo C startup/runtime-library data the port does not use; '
        'no bytes read or emitted for these).',
        f'- game_data.c: {data_report["emitted"]} objects emitted, '
        f'{data_report["skipped_owned"]} skipped (subsystem-owned BSS aliasing a DATA base -- '
        'should not normally happen, DATA is always generated), '
        f'{data_report["skipped_code_owned"]} components skipped (ported-C owned), '
        f'{data_report["skipped_toolchain"]} components skipped (toolchain-opaque, no bytes).',
        f'- game_state.c: {state_report["emitted"]} objects emitted '
        f'({state_report["typed"]} typed, {state_report["untyped"]} untyped uint8_t fallback), '
        f'{state_report["skipped_owned"]} skipped (subsystem-owned).',
        f'- Symbol names merged from all six sources: {len(table.offset_by_name)}, '
        f'covering {len(table.names_by_offset)} distinct DGROUP offsets.',
        f'- Offset-ambiguous names recorded during the merge (same name, two different '
        f'offsets across sources -- the earlier-added source won): {len(table.conflicts)}.',
        f'- Alias type conflicts (same offset, incompatible declared types across its names): '
        f'{sum(1 for s in symbols if s["conflict"])}.',
        f'- Derivation warnings: {len(warnings)}.',
        '',
    ]

    lines += ['## EXE cross-check (optional)', '']
    if not exe_check['checked']:
        lines += [f"assets/AEPROG.EXE was not used: {exe_check['reason']}. Generation succeeds "
                   'identically either way -- the JSON recipe sources are the primary and only '
                   'byte source.', '']
    else:
        lines += [f"assets/AEPROG.EXE was present and matched layout/manifest.json; "
                   f"{exe_check['components_checked']} recipe components / "
                   f"{exe_check['bytes_checked']} non-pointer bytes were compared against it "
                   f"byte-for-byte and {len(exe_check['mismatches'])} mismatched "
                   f"({exe_check['mismatches'] if exe_check['mismatches'] else 'none'}). This is "
                   'purely a confidence check; it never supplies a byte the JSON sources did not.',
                   '']

    leading_gap = next((g for g in notes['gaps'] if g['ds_offset'] == 0), None)
    lines += ['## DS:0000..0x0092 inspection', '']
    if leading_gap is None:
        lines += ['No toolchain-opaque region starts at DS:0000 (layout assumption changed -- '
                   'investigate).', '']
    else:
        named_inside = [s for s in symbols if 0 <= s['offset'] < leading_gap['length']]
        if named_inside:
            lines += [f"{len(named_inside)} symbol(s) fall inside the leading "
                      f"{leading_gap['length']}-byte toolchain-opaque region "
                      f"(`{leading_gap['id']}`, {leading_gap['classification']}) -- unexpected; "
                      'see the matching entry under "Derivation warnings" above.', '']
            for s in named_inside:
                lines.append(f"- `{s['primary']}` at DS:{s['offset']:04x}")
            lines.append('')
        else:
            lines += [f"As expected: no symbol from any of the six sources names any offset in "
                      f"[0x0000, {leading_gap['length']:#06x}) (`{leading_gap['id']}`, "
                      f"{leading_gap['classification']}). Nothing in the ported game references "
                      'this Turbo C startup banner/error-text block, so it is entirely unmodeled.',
                      '']

    if any(s['conflict'] for s in symbols):
        lines += ['## Alias type conflicts', '',
                  "Several historical names resolve to the same DGROUP offset with types that "
                  "aren't safely interchangeable (different dos_* width/pointer-ness); the widest/"
                  "most-structured one was used for the generated object, and the others are left "
                  'as a comment (not a `#define`) pending supervisor review.', '',
                  '| offset | chosen name : type | other name : type (file:line) |',
                  '|---|---|---|']
        for s in sorted(symbols, key=lambda s: s['offset']):
            if not s['conflict']:
                continue
            for r in s['alias_type_reports']:
                if r['compatible']:
                    continue
                lines.append(f"| {s['offset']:#06x} | `{s['primary']}` : {s['c_type']} "
                             f"| `{r['name']}` : `{r['type_text']}` ({r['file']}:{r['line']}) |")
        lines.append('')

    if extra['segment_half']:
        lines += ['## Segment-half aliases (rule A)', '',
                   'A name sitting exactly 2 bytes after a 4-byte historical far pointer (or '
                   'interrupt-vector function pointer) is that pointer\'s own historical SEGMENT '
                   'word, not a separate object -- the pointer is emitted as a real (portable, '
                   'native-width) pointer and the +2 name gets no object of its own.', '']
        for entry in extra['segment_half']:
            for seg_name in entry['segment_names']:
                lines.append(f"- `{seg_name}`: historical segment word of "
                              f"`{entry['pointer_name']}` (DS:{entry['pointer_offset']:04x}) at "
                              f"DS:{entry['segment_offset']:04x}; no portable object.")
        lines.append('')

    if extra['interior_alias']:
        lines += ['## Interior aliases (rule B)', '',
                   'A name landing inside a fully-dimensioned declared array/struct type: the '
                   'declared type wins (emitted whole) and the interior name becomes an alias '
                   'EXPRESSION macro instead of a second, overlapping definition.', '',
                   '| name | DS offset | array | expression |', '|---|---|---|---|']
        for ia in sorted(extra['interior_alias'], key=lambda ia: ia['offset']):
            lines.append(f"| `{ia['name']}` | {ia['offset']:#06x} | `{ia['array_name']}` "
                         f"(`{ia['c_type']}`) | `{ia['expr']}` |")
        lines.append('')

    if extra['short_aliases']:
        lines += ['## Short aliases (no macro; use the primary name)', '',
                   'A `#define` alias is an OBJECT-LIKE macro: it rewrites every occurrence of its '
                   'name, not just uses meant as this alias, in any ported .c file that includes '
                   'the header -- dangerous for a short, unqualified name (`f1`, `f2`, `t3`, `t4`, '
                   '`err`, `cur`, `tbl`, `off`, ...) that easily collides with a struct field or a '
                   'local variable spelled the same way elsewhere. Names under 4 characters, and '
                   'any name (of any length) that equals a real struct field in '
                   '`portable/include/game_structs.h`/`game_funcs.h`, get no macro at all -- use '
                   'the primary name/expression directly instead.', '',
                   '| alias | primary name / expression | reason |', '|---|---|---|']
        for e in sorted(extra['short_aliases'], key=lambda e: e['name']):
            lines.append(f"| `{e['name']}` | `{e['target']}` | {e['reason']} |")
        lines.append('')

    if extra['ceil_arrays']:
        lines += ['## Struct arrays rounded up (rule E)', '',
                   "The measured span wasn't a whole number of records; the historical code only "
                   'ever reads whole records, so the count was rounded UP and the tail now '
                   'overlaps the next object\'s leading bytes (documented, not corrected -- the '
                   'DATA image already supplies real bytes there).', '',
                   '| name | DS offset | measured | record size | count | emitted size | overlap |',
                   '|---|---|---|---|---|---|---|']
        for e in sorted(extra['ceil_arrays'], key=lambda e: e['offset']):
            lines.append(f"| `{e['name']}` | {e['offset']:#06x} | {e['measured']} "
                         f"| {e['elem_bytes']} (`{e['c_type']}`) | {e['count']} | {e['new_size']} "
                         f"| {e['overlap']} byte(s) duplicate DS:{e['overlap_at']:04x} |")
        lines.append('')

    if extra['floor_arrays']:
        lines += ['## Undimensioned arrays rounded down (rule D)', '',
                   "The measured span wasn't a whole number of elements; floored to the widest "
                   'whole count and the trailing partial element is left unnamed/unclaimed.', '',
                   '| name | DS offset | measured | element size | count | leftover bytes |',
                   '|---|---|---|---|---|---|']
        for e in sorted(extra['floor_arrays'], key=lambda e: e['offset']):
            lines.append(f"| `{e['name']}` | {e['offset']:#06x} | {e['measured']} "
                         f"| {e['elem_bytes']} (`{e['c_type']}`) | {e['count']} | {e['leftover']} |")
        lines.append('')

    if extra['storage_alias']:
        lines += ['## Storage aliases (rule F)', '',
                   'A declared type genuinely overlaps a neighboring struct-shaped recipe '
                   "component's storage -- confirmed historical aliasing, not a data-quality "
                   'error. One raw-byte union object is emitted for the combined span; each '
                   'original name is a macro cast into it.', '',
                   '| union object | DS offset | size | member names |', '|---|---|---|---|']
        for u in extra['storage_alias']:
            members = ', '.join(f'`{m}`' for m in u['members'])
            lines.append(f"| `{u['union_name']}` | {u['offset']:#06x} | {u['size']} | {members} |")
        lines.append('')

    if extra['porting_notes']:
        lines += ['## Porting notes', ''] + [f'- {n}' for n in extra['porting_notes']] + ['']

    if table.conflicts:
        lines += ['## Offset-ambiguous names', '',
                   'A name resolved to two different DGROUP offsets from different sources; '
                   'the earliest-added offset won and the later one was dropped for that name. '
                   'Needs supervisor review.', '',
                   '| name | offsets | source of later occurrence |', '|---|---|---|']
        for c in table.conflicts:
            offs = ', '.join(f'{o:#06x}' for o in c['offsets'])
            lines.append(f"| `{c['name']}` | {offs} | {c['tag']} |")
        lines.append('')

    if warnings:
        lines += ['## Derivation warnings', ''] + [f'- {w}' for w in warnings] + ['']

    lines += ['## Symbols', '',
              '| primary name | section | DS offset | size | C type | definition | aliases |',
              '|---|---|---|---|---|---|---|']
    for s in sorted(symbols, key=lambda s: s['offset']):
        ctype = s['c_type'] or ''
        aliases = ', '.join(f'`{a}`' for a in s['aliases']) if s['aliases'] else ''
        flag = ' :warning: conflict' if s['conflict'] else ''
        lines.append(f"| `{s['primary']}` | {s['section']} | {s['offset']:#06x} | {s['size']} "
                      f"| {ctype}{flag} | {s['definition_site']} | {aliases} |")
    lines.append('')

    untyped = [s for s in symbols if s['definition_site'] == 'generated' and s['untyped']]
    lines += ['## Untyped symbols (supervisor: please resolve)', '',
              f'{len(untyped)} generated objects have no historical-type evidence '
              '(typed_reserves/extern comment) and fall back to a plain byte array. Each '
              'listing includes any note explaining *why* (e.g. "no portable struct definition '
              'for `struct foo` yet").', '',
              '| name | section | DS offset | size | note |', '|---|---|---|---|---|']
    for s in sorted(untyped, key=lambda s: s['offset']):
        lines.append(f"| `{s['primary']}` | {s['section']} | {s['offset']:#06x} | {s['size']} "
                      f"| {s['type_note'] or ''} |")
    lines.append('')

    lines += ['## Toolchain/alignment bytes not modeled', '',
              'Present in `assets/AEPROG.EXE` inside the DATA range but absent from '
              '`recipes/data/game-initialized.json` (Turbo C startup banner/runtime-error '
              'text, or CC.LIB runtime-library initialized data, or pure alignment padding). '
              'No ported C file references any of these by name, so datagen.py accounts for '
              'their length (to keep the 0x3902 total exact) without emitting a C object.', '',
              '| component id | DS offset | length | classification |', '|---|---|---|---|']
    for g in sorted(notes['gaps'], key=lambda g: g['ds_offset']):
        lines.append(f"| `{g['id']}` | {g['ds_offset']:#06x} | {g['length']} "
                      f"| {g['classification']} |")
    lines.append('')

    code_owned = [c for c in components if c['kind'] == 'code_owned']
    lines += ['## Ported-C-owned DATA (compiled-data components)', '',
              'These bytes come from Turbo C static initializers inside a `src/*.C` file; the '
              'ported C translation unit is expected to reproduce them byte-for-byte with its '
              'own initializer, so datagen.py never emits a definition here -- only the length, '
              'to keep offsets correct.', '',
              '| component id | DS offset | length | historical C file (code_owner) |',
              '|---|---|---|---|']
    for c in sorted(code_owned, key=lambda c: c['ds_offset']):
        lines.append(f"| `{c['id']}` | {c['ds_offset']:#06x} | {c['length']} "
                      f"| `{c['code_owner']}` |")
    lines.append('')

    ported_c_owned = data_report.get('ported_c_owned', [])
    if ported_c_owned:
        lines += ['Point 5: every top-level object `portable/game/*.c` DEFINES '
                  '(scan_ported_c_definitions) that the generator does not already emit under '
                  'that same name gets an `extern` declaration in game_data.h (never a second '
                  'definition) so other ported units can see it -- regardless of whether any of '
                  'the 7 symbol sources happens to know its DS offset.', '',
                  '| name | DS offset | component id | defined in |',
                  '|---|---|---|---|']
        for e in sorted(ported_c_owned, key=lambda e: (e['offset'] is None, e['offset'] or 0)):
            offset_cell = f"{e['offset']:#06x}" if e['offset'] is not None else '(unknown)'
            component_cell = f"`{e['component_id']}`" if e['component_id'] else '(unknown)'
            lines.append(f"| `{e['name']}` | {offset_cell} | {component_cell} | `{e['file']}` |")
        lines.append('')

    out_path.write_text('\n'.join(lines), encoding='utf-8', newline='\n')


def generate(out_generated=None, out_docs=None, verbose=True):
    out_generated = out_generated or GENERATED_DIR
    out_docs = out_docs or DOCS_DIR

    image, components, notes = build_component_map()
    assert len(image) == DATA_LEN
    exe_check = cross_check_against_exe(image, components)

    externs = parse_all_externs()
    ic_offsets, ic_typed_entries = load_interface_conflicts()
    # Source (vii) is an independent OMF/interface-census scan: it also
    # finds genuine Turbo C runtime-library internals (atexit counter,
    # malloc's free-list head, the ctype table, the open-file-descriptor
    # table, ...) that happen to fall inside a toolchain_opaque region --
    # correctly unmodeled (no recipe bytes, no portable meaning) before
    # this source existed. Naming them would only produce a "name falls
    # inside an opaque region, nothing to emit" warning for something
    # nobody ported and nothing points at; drop any (vii) entry whose
    # offset lands in a toolchain_opaque/code_owned region instead.
    _opaque_ranges = [(c['ds_offset'], c['ds_offset'] + c['length'])
                       for c in components if c['kind'] in ('toolchain_opaque', 'code_owned')]

    def _in_opaque_region(off):
        return any(lo <= off < hi for lo, hi in _opaque_ranges)

    ic_offsets = [(n, o) for n, o in ic_offsets if not _in_opaque_region(o)]
    ic_typed_entries = {n: entries for n, entries in ic_typed_entries.items()
                         if not _in_opaque_region(entries[0]['offset'])}
    for name, entries in ic_typed_entries.items():
        externs.setdefault(name, []).extend(entries)
    table, friendly = build_symbol_table(components, externs, ic_offsets)
    ownership = load_state_ownership()
    overrides = load_datagen_overrides()
    symbols, warnings, extra = resolve_symbols(components, table, friendly, externs, ownership,
                                                overrides)

    qualifiers = load_emit_qualifiers()
    data_report = emit_game_data(image, components, symbols, table, externs, extra, qualifiers,
                                  friendly, out_generated)
    state_report = emit_game_state(symbols, extra, qualifiers, out_generated)

    symbols_doc = write_symbols_json(components, symbols, table, notes,
                                      out_generated / 'symbols.json')
    out_docs.mkdir(parents=True, exist_ok=True)
    write_state_map_md(components, symbols, table, notes, warnings, data_report, state_report,
                        exe_check, extra, out_docs / 'state-map.md')

    report = {
        'data_len_ok': len(image) == DATA_LEN,
        'exe_cross_check': exe_check,
        'components': len(components),
        'gaps': len(notes['gaps']),
        'symbol_names': len(table.offset_by_name),
        'symbol_offsets': len(table.names_by_offset),
        'offset_conflicts': len(table.conflicts),
        'type_conflicts': sum(1 for s in symbols if s['conflict']),
        'warnings': len(warnings),
        'game_data': {k: v for k, v in data_report.items() if k != 'verify_bytes'},
        'game_state': state_report,
        'pointer_refs_total': sum(len(c.get('refs', [])) for c in components),
    }
    if verbose:
        print(json.dumps(report, indent=2))
    return report, {
        'image': image, 'components': components, 'notes': notes, 'table': table,
        'symbols': symbols, 'warnings': warnings, 'verify_bytes': data_report['verify_bytes'],
        'symbols_doc': symbols_doc, 'externs': externs, 'exe_check': exe_check, 'extra': extra,
    }


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--out-root', type=Path, default=None,
                         help='Alternate repo root to write portable/generated and '
                              'docs/portable under (default: this checkout).')
    args = parser.parse_args(argv)
    if args.out_root is not None:
        out_generated = args.out_root / 'portable/generated'
        out_docs = args.out_root / 'docs/portable'
    else:
        out_generated = None
        out_docs = None
    generate(out_generated, out_docs)


if __name__ == '__main__':
    main()
