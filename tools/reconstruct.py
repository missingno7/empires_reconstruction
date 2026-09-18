"""Compile owned C/ASM regions, bind fresh OMF objects, reconstruct and verify."""
import argparse
from collections import Counter
from dataclasses import asdict
import hashlib
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import sys
import tempfile

from mz import MZ
from omf import OmfReader

ROOT = Path(__file__).resolve().parents[1]


def sha(data):
    return hashlib.sha256(data).hexdigest()


def read_json(path):
    return json.loads(path.read_text(encoding='utf-8'))


def write_json(path, data):
    path.write_text(json.dumps(data, indent=2) + '\n', encoding='utf-8')


def project_path(root, name):
    path = (root / name).resolve()
    if not path.is_relative_to(root.resolve()):
        raise ValueError(f'Path escapes project: {name}')
    return path


def validate_layout(manifest):
    if manifest['format'] != 'empires-owned-exe-v1':
        raise ValueError('Unknown manifest format')
    cursor, ids = 0, set()
    for owner in manifest['regions']:
        name, start, end = owner['id'], owner['start'], owner['end']
        if name in ids:
            raise ValueError(f'Duplicate owner {name}')
        ids.add(name)
        if start != cursor:
            defect = 'overlap' if start < cursor else 'gap'
            raise ValueError(f'Ownership {defect} at file 0x{cursor:X}: {name} starts 0x{start:X}')
        if end <= start or end > manifest['original']['size']:
            raise ValueError(f'{name}: invalid range size')
        if owner['kind'] not in ('RAW', 'MATCHING_C', 'MATCHING_ASM', 'EXACT_DATA'):
            raise ValueError(f'{name}: unsupported owner kind')
        cursor = end
    if cursor != manifest['original']['size']:
        raise ValueError(f'Ownership gap at file 0x{cursor:X} through EOF')


def mismatch(expected, actual, owner, base=None):
    base = owner['start'] if base is None else base
    for i, (a, b) in enumerate(zip(expected, actual)):
        if a != b:
            raise ValueError(f"First mismatch file 0x{base+i:06X}, owner {owner['id']} "
                             f"({owner['kind']}, {owner['source']}), region +0x{i:X}: "
                             f'original={a:02X}, reconstructed={b:02X}')
    if len(expected) != len(actual):
        raise ValueError(f"{owner['id']}: wrong output length {len(actual)}, expected {len(expected)}; "
                         f'first missing/extra byte file 0x{base+min(len(expected),len(actual)):06X}')


def compile_sources(root, owners, work, toolchain, dosbox, lock):
    """One isolated DOS session. No cached OBJ can satisfy a fresh build."""
    tc = work / 'TC/BIN'
    units = work / 'WORK'
    tc.mkdir(parents=True)
    units.mkdir()
    for item in lock['files']:
        src = toolchain / item['path']
        if sha(src.read_bytes()) != item['sha256']:
            raise ValueError(f'Toolchain identity mismatch: {src}')
        shutil.copyfile(src, tc / src.name)
    commands = ['@echo off', 'c:', 'cd \\work', 'set PATH=C:\\TC\\BIN']
    receipts = {}
    for i, owner in enumerate(owners):
        stem = f'R{i:04d}'
        suffix = '.C' if owner['kind'] == 'MATCHING_C' else '.ASM'
        source = project_path(root, owner['source']).read_bytes()
        # Reproduce the upstream staging convention, without changing the canonical source.
        staged = source.decode('latin1').replace('\r\n', '\n').replace('\r', '\n').replace('\n', '\r\n').encode('latin1')
        (units / (stem + suffix)).write_bytes(staged)
        flags = lock['flags'] + ' ' + owner['build'].get('flags_append', '')
        if owner['kind'] == 'MATCHING_C':
            command = f'tcc {flags.strip()} {stem}.C'
        else:
            command = f'tasm /mx {stem}.ASM'
        commands += [f'echo {owner["id"]}>>BUILD.LOG', command + ' >> BUILD.LOG',
                     'if errorlevel 1 goto failed']
        receipts[owner['id']] = {'command': command, 'object': f'WORK/{stem}.OBJ',
                                 'source_sha256': sha(source), 'staged_sha256': sha(staged)}
    commands += ['echo OK>SUCCESS.TXT', 'goto done', ':failed', 'echo FAILED>FAILED.TXT', ':done']
    (work / 'GO.BAT').write_bytes(('\r\n'.join(commands) + '\r\n').encode('ascii'))
    conf = '\n'.join([
        '[sdl]', 'output=texture', 'window_position=2550,1430', 'window_size=320x200',
        '[mixer]', 'nosound=true', '[dosbox]', 'machine=svga_s3', 'memsize=16',
        '[cpu]', 'core=auto', 'cputype=auto', 'cycles=max', '[autoexec]',
        f'mount c "{work}"', 'c:', 'call c:\\go.bat', 'exit', ''])
    config = work / 'dosbox.conf'
    config.write_text(conf, encoding='utf-8')
    command = [str(dosbox), '-conf', str(config), '--noprimaryconfig', '-noconsole', '-exit']
    options = {}
    if os.name == 'nt':
        startup = subprocess.STARTUPINFO()
        startup.dwFlags |= subprocess.STARTF_USESHOWWINDOW
        startup.wShowWindow = 0
        options['startupinfo'] = startup
        options['creationflags'] = subprocess.CREATE_NO_WINDOW
    print(f'Compiling {len(owners)} source regions with Turbo C / TASM...', flush=True)
    with (work / 'host.log').open('wb') as log:
        result = subprocess.run(command, cwd=work, stdout=log, stderr=subprocess.STDOUT,
                                timeout=600, **options)
    if result.returncode or not (units / 'SUCCESS.TXT').exists():
        raise ValueError(f'Compiler session failed; inspect {units / "BUILD.LOG"} and {work / "host.log"}')
    for receipt in receipts.values():
        path = work / receipt['object']
        receipt['object_sha256'] = sha(path.read_bytes())
    return receipts, {'command': command, 'dosbox_sha256': sha(dosbox.read_bytes()),
                      'config_sha256': sha(config.read_bytes()), 'toolchain': lock}


def read_object(data):
    # The reused reader did not check record framing/checksums. Refuse corruption here.
    at, ended = 0, False
    allowed = {0x80, 0x82, 0x88, 0x8A, 0x8C, 0x90, 0x94, 0x96, 0x98, 0x9A, 0x9C, 0xA0, 0xA2, 0xB0, 0xB4, 0xB6}
    while at < len(data):
        if at + 3 > len(data):
            raise ValueError('Truncated OMF record header')
        kind, length = data[at], struct.unpack_from('<H', data, at + 1)[0]
        end = at + 3 + length
        if ended or length < 1 or end > len(data) or kind not in allowed:
            raise ValueError(f'Invalid/unsupported OMF record 0x{kind:X} at {at}')
        # OMF permits checksum zero to mean "not supplied".
        if data[end - 1] and sum(data[at:end]) & 255:
            raise ValueError(f'OMF checksum mismatch at {at}')
        ended = kind == 0x8A
        at = end
    if not ended:
        raise ValueError('OMF missing MODEND')
    return OmfReader().read(data)


def bind_region(owner, module, mz, frames):
    """Bind only from source OBJ and declared metadata; never receives original bytes."""
    build = owner['build']
    segment = build['segment']
    data = module.segment_bytes(segment)
    publics = module.publics_in(segment)
    index = next((i for i, p in enumerate(publics) if p['name'] == build['public']), None)
    if index is None:
        raise ValueError(f"{owner['id']}: missing public {build['public']}")
    start = publics[index]['offset']
    following = [p['offset'] for p in publics[index + 1:] if p['offset'] > start]
    span = build.get('span', 1)
    if span < 1:
        raise ValueError('Public span must be positive')
    end = following[span-1] if span <= len(following) else len(data)
    if len(data) != module.segment_length(segment):
        raise ValueError(f"{owner['id']}: emitted code does not cover SEGDEF length")
    result = bytearray(data[start:end])
    if len(result) != owner['end'] - owner['start']:
        raise ValueError(f"{owner['id']}: wrong compiled extent length {len(result)}, expected {owner['end']-owner['start']}")
    load_base = mz.load_offset(owner['start'])
    module_base = load_base - start
    relocations, patches, occupied = [], [], set()
    for f in module.fixups_in(segment):
        offset, width = f['offset'] - start, f['width']
        if offset + width <= 0 or offset >= len(result):
            continue
        if offset < 0 or offset + width > len(result):
            raise ValueError(f"{owner['id']}: fixup straddles extent boundary")
        positions = set(range(offset, offset + width))
        if occupied & positions:
            raise ValueError('Overlapping OMF fixups')
        occupied |= positions
        symbol, kind = f['target'], f['target_kind']
        if f['loc'] == 'pointer32' and width == 4 and not f['self_relative']:
            if kind != 'external' or symbol not in build['bindings']:
                raise ValueError(f'Undeclared far pointer {symbol}')
            binding = build['bindings'][symbol]
            if binding['coordinate'] != 'code_offset':
                raise ValueError(f'Unsupported far data pointer {symbol}')
            addend = int.from_bytes(result[offset:offset+2], 'little')
            segment_addend = int.from_bytes(result[offset+2:offset+4], 'little')
            value = (binding['offset'] + f['displacement'] + addend) & 0xFFFF
            segment_value = (frames[segment] // 16 + segment_addend) & 0xFFFF
            result[offset:offset+4] = struct.pack('<HH', value, segment_value)
            relocations.append(load_base + offset + 2)
            patches.append(dict(f, extent_offset=offset, addend=addend,
                                resolved_value=value, segment_value=segment_value))
            continue
        if f['loc'] == 'base16' and width == 2 and not f['self_relative']:
            if kind not in ('group', 'segment') or symbol not in frames:
                raise ValueError(f'Undeclared segment base {symbol}')
            value = frames[symbol] // 16
            if frames[symbol] % 16:
                raise ValueError('Segment base is not paragraph aligned')
            relocations.append(load_base + offset)
        elif f['loc'] == 'offset16' and width == 2:
            if kind == 'segment' and symbol == segment:
                value = module_base
            elif kind == 'segment' and symbol in build['module_segments']:
                value = build['module_segments'][symbol]['offset']
            elif kind == 'external' and symbol in build['bindings']:
                binding = build['bindings'][symbol]
                if f['self_relative'] and binding['coordinate'] != 'code_offset':
                    raise ValueError('Self-relative fixup requires a code address')
                value = binding['offset']
            else:
                raise ValueError(f"{owner['id']}: unresolved {kind} target {symbol}")
            if f['self_relative']:
                value -= load_base + offset + width
        else:
            raise ValueError(f"{owner['id']}: unsupported fixup {f}")
        addend = int.from_bytes(result[offset:offset+width], 'little')
        value = (value + f['displacement'] + addend) & 0xFFFF
        result[offset:offset+width] = value.to_bytes(width, 'little')
        patches.append(dict(f, extent_offset=offset, addend=addend, resolved_value=value))
    expected_relocations = sorted(r['load_offset'] for r in mz.relocations
                                  if load_base <= r['load_offset'] < load_base + len(result))
    if sorted(relocations) != expected_relocations:
        raise ValueError(f"{owner['id']}: source relocation map differs: {relocations} != {expected_relocations}")
    return bytes(result), {'object_span': [start, end], 'module_load_base': module_base,
                           'fixups': patches, 'load_relocations': relocations}


def reconstruct(root, manifest_path, output, toolchain, dosbox):
    output.mkdir(parents=True, exist_ok=True)
    # Invalidate old success before even checking inputs, including manifest errors.
    for name in ('AEPROG.EXE', 'report.json'):
        (output / name).unlink(missing_ok=True)
    manifest = read_json(manifest_path)
    validate_layout(manifest)
    original = project_path(root, manifest['original']['path']).read_bytes()
    if len(original) != manifest['original']['size'] or sha(original) != manifest['original']['sha256']:
        raise ValueError('Original EXE identity does not match the manifest')
    mz = MZ.parse(original)
    sources = [r for r in manifest['regions'] if r['kind'] in ('MATCHING_C', 'MATCHING_ASM')]
    work = Path(tempfile.mkdtemp(prefix='session-', dir=output)).resolve()
    receipts, session = {}, None
    if sources:
        lock = read_json(root / 'layout/toolchain.json')
        receipts, session = compile_sources(root, sources, work, toolchain, dosbox, lock)
    image, reports = bytearray(), []
    for owner in manifest['regions']:
        proof = {}
        if owner['kind'] in ('RAW', 'EXACT_DATA'):
            part = project_path(root, owner['source']).read_bytes()
        else:
            receipt = receipts[owner['id']]
            module = read_object((work / receipt['object']).read_bytes())
            part, proof = bind_region(owner, module, mz, manifest['frames'])
            artifact = project_path(output, owner['artifact'])
            artifact.parent.mkdir(parents=True, exist_ok=True)
            artifact.write_bytes(part)
            proof['compile'] = receipt
        mismatch(original[owner['start']:owner['end']], part, owner)
        if sha(part) != owner['expected_sha256']:
            raise ValueError(f"{owner['id']}: manifest extent digest differs")
        image.extend(part)
        reports.append({'id': owner['id'], 'kind': owner['kind'], 'start': owner['start'],
                        'end': owner['end'], 'sha256': sha(part), 'status': 'EQUAL', **proof})
    rebuilt = bytes(image)
    rebuilt_mz = MZ.parse(rebuilt)
    checks = {'load_image': mz.load_image(original) == rebuilt_mz.load_image(rebuilt),
              'relocations': mz.relocations == rebuilt_mz.relocations and mz.relocation_bytes(original) == rebuilt_mz.relocation_bytes(rebuilt),
              'full_exe': original == rebuilt}
    if not all(checks.values()):
        raise ValueError(f'Whole-file verification failed: {checks}')
    counts = Counter()
    for r in manifest['regions']:
        counts[r['kind']] += r['end'] - r['start']
    (output / 'AEPROG.EXE').write_bytes(rebuilt)
    report = {'status': 'EQUAL', 'checks': checks, 'original_sha256': sha(original),
              'reconstructed_sha256': sha(rebuilt), 'total_bytes': len(rebuilt),
              'bytes_by_kind': dict(counts), 'mz': asdict(mz),
              'manifest_sha256': sha(manifest_path.read_bytes()),
              'builder_sha256': sha(Path(__file__).read_bytes()),
              'omf_reader_sha256': sha((root / 'tools/omf.py').read_bytes()),
              'session_directory': str(work), 'session': session, 'regions': reports}
    write_json(output / 'report.json', report)
    print(f'Total executable bytes: {len(rebuilt):,}\nAccounted bytes:        100% ({len(manifest["regions"])} owners)')
    for kind in ('MATCHING_C', 'MATCHING_ASM', 'RAW', 'EXACT_DATA'):
        print(f'{kind + " bytes:":24s}{counts[kind]:,}')
    print('Load image match:       PASS\nRelocation match:       PASS\nFull EXE match:         PASS')
    print(f'Original SHA256:        {sha(original)}\nReconstructed SHA256:   {sha(rebuilt)}')
    print(f'Output: {output / "AEPROG.EXE"}\nReport: {output / "report.json"}')
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--manifest', type=Path, default=ROOT / 'layout/manifest.json')
    parser.add_argument('--output', type=Path, default=ROOT / 'build')
    parser.add_argument('--toolchain', type=Path, default=ROOT / 'toolchain')
    parser.add_argument('--dosbox', type=Path, default=Path(os.environ.get('DOSBOX', 'C:/Program Files/DOSBox Staging/dosbox.exe')))
    args = parser.parse_args()
    # Output must stay in the generated build tree, never in assets or source directories.
    output = args.output.resolve()
    if not output.is_relative_to((ROOT / 'build').resolve()):
        parser.error('--output must be within the project build directory')
    try:
        reconstruct(ROOT, args.manifest.resolve(), output, args.toolchain.resolve(), args.dosbox.resolve())
    except (ValueError, OSError, KeyError, subprocess.SubprocessError) as error:
        print(f'FAIL: {error}', file=sys.stderr)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
