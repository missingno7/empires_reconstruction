"""Build the historical oracle harness: ORACLE.EXE.

Assembles asm/DECODE.ASM, asm/RUNTIME_BLOCK.ASM and DSCALL.ASM with the
pinned TASM, compiles ORACLE.C with the pinned TCC (compact model), and
links everything with the pinned TLINK against C0C.OBJ/CC.LIB.

asm/RUNTIME_BLOCK.ASM is position-dependent (`RT_CS equ 039Ch`): its
CS-relative jump/dispatch tables are only correct if the module's _TEXT
contribution starts at physical offset 0x039C within the linked program's
code segment.  This script links once to measure where RUNTIME_BLOCK.OBJ
actually landed (from OUT.MAP), inserts a padding _TEXT object of the
right size before it (tools/omf_scaffold.make_text_padding), and
re-links -- repeating until the measured offset is exactly 0x039C.

Usage: python tools/portable/oracle/build_oracle.py
Produces build/oracle/ORACLE.EXE (and leaves the link session under
build/oracle/work/ for inspection).
"""
import hashlib
import re
import shutil
import struct
import sys
from pathlib import Path

ORACLE_DIR = Path(__file__).resolve().parent
ROOT = ORACLE_DIR.parents[2]
sys.path.insert(0, str(ROOT / 'tools'))

from reconstruct import dos_text, read_json, sha          # noqa: E402
from dos_runner import resolve_runner                       # noqa: E402
from omf_scaffold import make_text_padding                  # noqa: E402
from link_support import link_errors                        # noqa: E402
from resource_codecs import decode_payload                  # noqa: E402

RUNTIME_BASE_TARGET = 0x039C
MAX_LINK_ATTEMPTS = 6

ASM_UNITS = ['DECODE.ASM', 'RUNTIME_BLOCK.ASM', 'DSCALL.ASM']

# ---------------------------------------------------------------------------
# VGA (display_mode 5) replacement runtime: AE000_002, decoded, wrapped in a
# generated TASM module with PUBLIC labels at the same jump-table offsets
# RUNTIME_BLOCK.ASM uses (docs/portable/reference/AE000_002-vga-runtime.lst).
# gfx_box (entry 1, offset 3) is included for completeness but never called
# (see ORACLE.C -- gfx_box/present writes VRAM and is out of scope).
# ---------------------------------------------------------------------------

AE000_002_SHA256 = '763d28e6049dee36e7be92a10201e64b0bf7d1f62d73d83736e215073e2ddf87'
AE000_002_LEN = 1886

VGA_ENTRY_OFFSETS = [
    (0x00, '_runtime_base'),
    (0x03, '_gfx_box'),
    (0x06, '_gfx_bar'),
    (0x09, '_gfx_vline'),
    (0x0C, '_gfx_clear_rect'),
    (0x0F, '_gfx_fill_rect'),
    (0x12, '_gfx_save_rect'),
    (0x15, '_gfx_restore_rect'),
    (0x18, '_gfx_wipe_rect'),
    (0x1B, '_gfx_copy_rect_flip_v'),
    (0x1E, '_gfx_copy_rect_flip_h'),
    (0x21, '_gfx_copy_rect_flip_hv'),
    (0x24, '_gfx_copy_rect_split'),
    (0x27, '_gfx_copy_rect_split_flip_v'),
    (0x2A, '_gfx_draw_char'),
    (0x2D, '_gfx_blit_bitmap'),
    (0x30, '_gfx_copy_rect'),
    (0x33, '_gfx_blit_image'),
    (0x36, '_gfx_set_pixel'),
    (0x39, '_gfx_get_pixel'),
]


def decode_ae000_002():
    """AE000_002: type 0x46, flags 3 (LZ then RLE), no sprite decode
    (type is not 0x47/0/1).  Decoded independently in Python
    (tools/resource_codecs.decode_payload) rather than via ORACLE.EXE --
    this only needs to run once at build time and the two are already
    cross-checked (all 220 AE000/AE001 records, this one included, match
    portable/tests/fixtures/resource_golden.json byte for byte)."""
    data = (ROOT / 'assets/AE000.DAT').read_bytes()
    idx = 2
    o1 = struct.unpack_from('<I', data, idx * 4)[0]
    o2 = struct.unpack_from('<I', data, (idx + 1) * 4)[0]
    raw = data[o1:o2]
    rtype, flags = raw[0], raw[1]
    decoded = decode_payload(raw[2:], flags)
    got = hashlib.sha256(decoded).hexdigest()
    if len(decoded) != AE000_002_LEN or got != AE000_002_SHA256:
        raise SystemExit(f'AE000_002 decode mismatch: len={len(decoded)} (want {AE000_002_LEN}), '
                         f'sha256={got} (want {AE000_002_SHA256})')
    return decoded, rtype, flags


def make_vga_runtime_asm(decoded):
    lines = [
        "_TEXT segment byte public 'CODE'",
        '_TEXT ends',
        "_DATA segment word public 'DATA'",
        '_DATA ends',
        "_BSS segment word public 'BSS'",
        '_BSS ends',
        'DGROUP group _DATA,_BSS',
        'assume cs:_TEXT,ds:DGROUP',
        "_TEXT segment byte public 'CODE'",
    ]
    all_names = [name for _, name in VGA_ENTRY_OFFSETS] + ['_runtime_block_end']
    for i in range(0, len(all_names), 4):
        lines.append('public ' + ','.join(all_names[i:i + 4]))
    for i, (off, name) in enumerate(VGA_ENTRY_OFFSETS):
        end = VGA_ENTRY_OFFSETS[i + 1][0] if i + 1 < len(VGA_ENTRY_OFFSETS) else len(decoded)
        lines.append(f'{name} label near')
        chunk = decoded[off:end]
        for j in range(0, len(chunk), 16):
            row = chunk[j:j + 16]
            lines.append(' db ' + ','.join(f'0{b:02X}h' for b in row))
    lines.append('_runtime_block_end label byte')
    lines.append('_TEXT ends')
    lines.append('end')
    return '\r\n'.join(lines) + '\r\n'


def _verify_toolchain(lock):
    for item in lock['files']:
        src = ROOT / 'toolchain' / item['path']
        if sha(src.read_bytes()) != item['sha256']:
            raise SystemExit(f'Toolchain identity mismatch: {src}')
    for item in lock['objects']:
        src = ROOT / 'toolchain' / item['path']
        if sha(src.read_bytes()) != item['sha256']:
            raise SystemExit(f'Toolchain identity mismatch: {src}')
    for item in lock['libraries']:
        src = ROOT / 'toolchain' / item['path']
        if sha(src.read_bytes()) != item['sha256']:
            raise SystemExit(f'Toolchain identity mismatch: {src}')
    for item in lock['linkers']:
        src = ROOT / 'toolchain' / item['path']
        if sha(src.read_bytes()) != item['sha256']:
            raise SystemExit(f'Toolchain identity mismatch: {src}')


def _stage(work, lock, asm_units=ASM_UNITS, extra_asm=()):
    for item in lock['files'] + lock['objects'] + lock['libraries'] + lock['linkers']:
        shutil.copyfile(ROOT / 'toolchain' / item['path'], work / item['path'])
    for name in asm_units:
        source = ROOT / 'asm' / name if name != 'DSCALL.ASM' else ORACLE_DIR / name
        (work / name).write_bytes(dos_text(source.read_bytes()))
    for name, text in extra_asm:
        (work / name).write_bytes(dos_text(text))
    (work / 'ORACLE.C').write_bytes(dos_text((ORACLE_DIR / 'ORACLE.C').read_bytes()))


def _run(runner, work, program, args, what):
    result, command, output = runner.run(work / program, args, work, timeout=180)
    log = work / (program.rsplit('.', 1)[0] + '.LOG')
    log.write_bytes(output)
    if result.returncode:
        raise SystemExit(f'{what} failed (exit {result.returncode}); see {log}\n'
                         f'--- tail ---\n{output[-2000:].decode("latin1", "replace")}')
    return output


def compile_and_assemble(runner, work, lock, asm_units=ASM_UNITS, driver_source='ORACLE.C',
                         driver_extra_flags=()):
    flags = lock['flags'].split() + list(driver_extra_flags)
    _run(runner, work, 'TCC.EXE', flags + [driver_source], f'TCC compile of {driver_source}')
    driver_obj = driver_source.rsplit('.', 1)[0] + '.OBJ'
    if not (work / driver_obj).exists():
        raise SystemExit(f'TCC did not produce {driver_obj}')
    for name in asm_units:
        _run(runner, work, 'TASM.EXE', ['/mx', name], f'TASM assembly of {name}')
        obj = name.rsplit('.', 1)[0] + '.OBJ'
        if not (work / obj).exists():
            raise SystemExit(f'TASM did not produce {obj}')


def _parse_map_publics(map_text):
    publics = {}
    for frame, offset, name in re.findall(r'^\s*([0-9A-F]+):([0-9A-F]+)\s+(\S+)\s*$', map_text, re.M):
        publics[name.upper()] = (int(frame, 16), int(offset, 16))
    return publics


def link(runner, work, pad_len, runtime_obj, driver_obj, exe_name, map_name):
    objects = ['C0C.OBJ']
    if pad_len:
        (work / 'PAD.OBJ').write_bytes(make_text_padding(pad_len, 'PAD'))
        objects.append('PAD.OBJ')
    objects += [runtime_obj, driver_obj, 'DECODE.OBJ', 'DSCALL.OBJ']
    response = '/s ' + '+'.join(objects) + f',{exe_name},{map_name},CC.LIB'
    (work / 'LINK.RSP').write_bytes(dos_text(response + '\n'))
    result, command, output = runner.run(work / 'TLINK.EXE', ['@LINK.RSP'], work, timeout=180)
    (work / 'LINK.LOG').write_bytes(output)
    map_text = (work / map_name).read_text(errors='replace') if (work / map_name).exists() else ''
    errors = link_errors(output.decode('latin1', 'replace'), map_text)
    if result.returncode or errors or not (work / exe_name).exists():
        raise SystemExit(f'TLINK failed (exit {result.returncode}): {errors}\n'
                         f'--- tail ---\n{output[-2000:].decode("latin1", "replace")}')
    return _parse_map_publics(map_text), map_text


def _link_with_padding(runner, work, runtime_obj, driver_obj, exe_name, map_name):
    """Shared by build() and build_vga(): both RUNTIME_BLOCK.ASM and the
    generated VGA runtime module are position-dependent at _TEXT+0x039C
    (see module docstring / make_vga_runtime_asm's PUBLIC _runtime_base)."""
    pad_len = 0
    for attempt in range(MAX_LINK_ATTEMPTS):
        publics, map_text = link(runner, work, pad_len, runtime_obj, driver_obj, exe_name, map_name)
        if '_RUNTIME_BASE' not in publics:
            raise SystemExit(f'_runtime_base not in {map_name} publics; attempt {attempt}\n{map_text[:4000]}')
        frame, offset = publics['_RUNTIME_BASE']
        if offset == RUNTIME_BASE_TARGET:
            print(f'{exe_name}: _runtime_base landed at offset 0x{offset:04X} after {attempt + 1} '
                 f'link attempt(s), pad_len=0x{pad_len:X}')
            return map_text
        delta = RUNTIME_BASE_TARGET - offset
        pad_len += delta
        if pad_len <= 0:
            raise SystemExit(f'_runtime_base at 0x{offset:04X} without padding already exceeds target 0x{RUNTIME_BASE_TARGET:04X}')
    raise SystemExit(f'_runtime_base did not converge to 0x{RUNTIME_BASE_TARGET:04X} within {MAX_LINK_ATTEMPTS} attempts '
                     f'(last offset 0x{offset:04X}, pad_len=0x{pad_len:X})')


def build(runner=None, dosbox=None):
    (ROOT / 'build/oracle').mkdir(parents=True, exist_ok=True)
    work = ROOT / 'build/oracle/work'
    if work.exists():
        shutil.rmtree(work)
    work.mkdir(parents=True)

    lock = read_json(ROOT / 'layout/toolchain.json')
    _verify_toolchain(lock)
    _stage(work, lock)

    runner = runner or resolve_runner(lock, backend='dosbox' if dosbox else None, executable=dosbox)
    compile_and_assemble(runner, work, lock)

    map_text = _link_with_padding(runner, work, 'RUNTIME_BLOCK.OBJ', 'ORACLE.OBJ', 'ORACLE.EXE', 'ORACLE.MAP')

    exe = work / 'ORACLE.EXE'
    out = ROOT / 'build/oracle/ORACLE.EXE'
    shutil.copyfile(exe, out)
    (ROOT / 'build/oracle/ORACLE.MAP').write_text(map_text, encoding='latin1')
    print(f'Built {out} ({out.stat().st_size} bytes), sha256={sha(out.read_bytes())}')
    excerpt = '\n'.join(line for line in map_text.splitlines() if '_RUNTIME_BASE' in line.upper() or '_RUNTIME_BLOCK_END' in line.upper())
    print('Map excerpt:\n' + excerpt)
    return out, map_text


def build_vga(runner=None, dosbox=None):
    """Build ORACLEV.EXE: same DECODE.ASM/DSCALL.ASM/ORACLE.C driver as
    ORACLE.EXE, but asm/RUNTIME_BLOCK.ASM is replaced by a generated
    module wrapping the decoded AE000_002 record (the display_mode 5 / VGA
    replacement runtime), and ORACLE.C is compiled with -DVGA_MODE (320
    bytes/row, 8bpp -- see the FB_STRIDE/FB_BYTES #ifdef in ORACLE.C)."""
    (ROOT / 'build/oracle').mkdir(parents=True, exist_ok=True)
    work = ROOT / 'build/oracle/workv'
    if work.exists():
        shutil.rmtree(work)
    work.mkdir(parents=True)

    lock = read_json(ROOT / 'layout/toolchain.json')
    _verify_toolchain(lock)
    decoded, rtype, flags = decode_ae000_002()
    vga_asm = make_vga_runtime_asm(decoded)
    _stage(work, lock, asm_units=['DECODE.ASM', 'DSCALL.ASM'],
          extra_asm=[('RUNTIME_VGA.ASM', vga_asm)])
    driver = (ORACLE_DIR / 'ORACLE.C').read_text(encoding='ascii')
    before = driver
    driver = driver.replace(
        '#define FB_STRIDE 0xA0u                /* 160 bytes/row, display_mode 4; VGA_STRIDE_MARKER */',
        '#define FB_STRIDE 0x140u               /* 320 bytes/row, display_mode 5; VGA_STRIDE_MARKER */')
    driver = driver.replace(
        '#define FB_BYTES  78080UL              /* FB_STRIDE * FB_ROWS; VGA_BYTES_MARKER */',
        '#define FB_BYTES  156160UL             /* FB_STRIDE * FB_ROWS; VGA_BYTES_MARKER */')
    if driver == before:
        raise SystemExit('build_vga: FB_STRIDE/FB_BYTES markers not found in ORACLE.C -- '
                         'source drifted from what this substitution expects')
    (work / 'ORACLEV.C').write_bytes(dos_text(driver))

    runner = runner or resolve_runner(lock, backend='dosbox' if dosbox else None, executable=dosbox)
    compile_and_assemble(runner, work, lock, asm_units=['DECODE.ASM', 'RUNTIME_VGA.ASM', 'DSCALL.ASM'],
                         driver_source='ORACLEV.C')

    map_text = _link_with_padding(runner, work, 'RUNTIME_VGA.OBJ', 'ORACLEV.OBJ', 'ORACLEV.EXE', 'ORACLEV.MAP')

    exe = work / 'ORACLEV.EXE'
    out = ROOT / 'build/oracle/ORACLEV.EXE'
    shutil.copyfile(exe, out)
    (ROOT / 'build/oracle/ORACLEV.MAP').write_text(map_text, encoding='latin1')
    print(f'Built {out} ({out.stat().st_size} bytes), sha256={sha(out.read_bytes())} '
         f'(AE000_002 type=0x{rtype:02X} flags=0x{flags:02X} decoded_len={len(decoded)})')
    excerpt = '\n'.join(line for line in map_text.splitlines() if '_RUNTIME_BASE' in line.upper() or '_RUNTIME_BLOCK_END' in line.upper())
    print('Map excerpt:\n' + excerpt)
    return out, map_text


if __name__ == '__main__':
    dosbox = '--dosbox' in sys.argv
    build(dosbox=dosbox)
    build_vga(dosbox=dosbox)
