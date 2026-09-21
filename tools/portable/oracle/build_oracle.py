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
import re
import shutil
import sys
from pathlib import Path

ORACLE_DIR = Path(__file__).resolve().parent
ROOT = ORACLE_DIR.parents[2]
sys.path.insert(0, str(ROOT / 'tools'))

from reconstruct import dos_text, read_json, sha          # noqa: E402
from dos_runner import resolve_runner                       # noqa: E402
from omf_scaffold import make_text_padding                  # noqa: E402
from link_support import link_errors                        # noqa: E402

RUNTIME_BASE_TARGET = 0x039C
MAX_LINK_ATTEMPTS = 6

ASM_UNITS = ['DECODE.ASM', 'RUNTIME_BLOCK.ASM', 'DSCALL.ASM']


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


def _stage(work, lock):
    for item in lock['files'] + lock['objects'] + lock['libraries'] + lock['linkers']:
        shutil.copyfile(ROOT / 'toolchain' / item['path'], work / item['path'])
    for name in ASM_UNITS:
        source = ROOT / 'asm' / name if name != 'DSCALL.ASM' else ORACLE_DIR / name
        (work / name).write_bytes(dos_text(source.read_bytes()))
    (work / 'ORACLE.C').write_bytes(dos_text((ORACLE_DIR / 'ORACLE.C').read_bytes()))


def _run(runner, work, program, args, what):
    result, command, output = runner.run(work / program, args, work, timeout=180)
    log = work / (program.rsplit('.', 1)[0] + '.LOG')
    log.write_bytes(output)
    if result.returncode:
        raise SystemExit(f'{what} failed (exit {result.returncode}); see {log}\n'
                         f'--- tail ---\n{output[-2000:].decode("latin1", "replace")}')
    return output


def compile_and_assemble(runner, work, lock):
    flags = lock['flags'].split()
    _run(runner, work, 'TCC.EXE', flags + ['ORACLE.C'], 'TCC compile of ORACLE.C')
    if not (work / 'ORACLE.OBJ').exists():
        raise SystemExit('TCC did not produce ORACLE.OBJ')
    for name in ASM_UNITS:
        _run(runner, work, 'TASM.EXE', ['/mx', name], f'TASM assembly of {name}')
        obj = name.rsplit('.', 1)[0] + '.OBJ'
        if not (work / obj).exists():
            raise SystemExit(f'TASM did not produce {obj}')


def _parse_map_publics(map_text):
    publics = {}
    for frame, offset, name in re.findall(r'^\s*([0-9A-F]+):([0-9A-F]+)\s+(\S+)\s*$', map_text, re.M):
        publics[name.upper()] = (int(frame, 16), int(offset, 16))
    return publics


def link(runner, work, pad_len):
    objects = ['C0C.OBJ']
    if pad_len:
        (work / 'PAD.OBJ').write_bytes(make_text_padding(pad_len, 'PAD'))
        objects.append('PAD.OBJ')
    objects += ['RUNTIME_BLOCK.OBJ', 'ORACLE.OBJ', 'DECODE.OBJ', 'DSCALL.OBJ']
    response = '/s ' + '+'.join(objects) + ',ORACLE.EXE,ORACLE.MAP,CC.LIB'
    (work / 'LINK.RSP').write_bytes(dos_text(response + '\n'))
    result, command, output = runner.run(work / 'TLINK.EXE', ['@LINK.RSP'], work, timeout=180)
    (work / 'LINK.LOG').write_bytes(output)
    map_text = (work / 'ORACLE.MAP').read_text(errors='replace') if (work / 'ORACLE.MAP').exists() else ''
    errors = link_errors(output.decode('latin1', 'replace'), map_text)
    if result.returncode or errors or not (work / 'ORACLE.EXE').exists():
        raise SystemExit(f'TLINK failed (exit {result.returncode}): {errors}\n'
                         f'--- tail ---\n{output[-2000:].decode("latin1", "replace")}')
    return _parse_map_publics(map_text), map_text


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

    pad_len = 0
    for attempt in range(MAX_LINK_ATTEMPTS):
        publics, map_text = link(runner, work, pad_len)
        if '_RUNTIME_BASE' not in publics:
            raise SystemExit(f'_runtime_base not in ORACLE.MAP publics; attempt {attempt}\n{map_text[:4000]}')
        frame, offset = publics['_RUNTIME_BASE']
        if offset == RUNTIME_BASE_TARGET:
            print(f'_runtime_base landed at offset 0x{offset:04X} after {attempt + 1} link attempt(s), pad_len=0x{pad_len:X}')
            break
        delta = RUNTIME_BASE_TARGET - offset
        pad_len += delta
        if pad_len <= 0:
            raise SystemExit(f'_runtime_base at 0x{offset:04X} without padding already exceeds target 0x{RUNTIME_BASE_TARGET:04X}')
    else:
        raise SystemExit(f'_runtime_base did not converge to 0x{RUNTIME_BASE_TARGET:04X} within {MAX_LINK_ATTEMPTS} attempts '
                         f'(last offset 0x{offset:04X}, pad_len=0x{pad_len:X})')

    exe = work / 'ORACLE.EXE'
    out = ROOT / 'build/oracle/ORACLE.EXE'
    shutil.copyfile(exe, out)
    (ROOT / 'build/oracle/ORACLE.MAP').write_text(map_text, encoding='latin1')
    print(f'Built {out} ({out.stat().st_size} bytes), sha256={sha(out.read_bytes())}')
    excerpt = '\n'.join(line for line in map_text.splitlines() if '_RUNTIME_BASE' in line.upper() or '_RUNTIME_BLOCK_END' in line.upper())
    print('Map excerpt:\n' + excerpt)
    return out, map_text


if __name__ == '__main__':
    dosbox = '--dosbox' in sys.argv
    build(dosbox=dosbox)
