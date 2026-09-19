"""Run a relocatable C/OMF scaffold through a local Borland TLINK.

This is an experiment path. It never changes the fixed manifest or canonical
EXE. It compiles the proven C owners after the compact startup extent, links
them with the pinned local C0C.OBJ and CC.LIB, and records the first address or
module-order divergence from the fixed oracle.
"""
import argparse
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile

from omf_scaffold import make_text_padding, trim_text_contribution
from reconstruct import ROOT, compile_sources, read_json, sha, write_json


DEFAULT_LINKER = Path(r'D:/Games/DOS/dos_recosystem/aladdin_forged/toolchain/dos/BC/BIN/TLINK.EXE')
CODE_ROW = re.compile(
    r'^\s*([0-9A-F]+):([0-9A-F]+)\s+([0-9A-F]+)\s+C=CODE\s+S=_TEXT\s+.*?M=(\S+)\s+ACBP=([0-9A-F]+)')
SEGMENT_ROW = re.compile(
    r'^\s*([0-9A-F]+)H\s+([0-9A-F]+)H\s+([0-9A-F]+)H\s+(\S+)\s+(\S+)')


def parse_map(path):
    rows, segments = [], []
    detailed = False
    for line in path.read_text(errors='replace').splitlines():
        if line.startswith('Detailed map of segments'):
            detailed = True
            continue
        if detailed:
            match = CODE_ROW.match(line)
            if match:
                segment, offset, length, module, acbp = match.groups()
                rows.append({'segment': int(segment, 16), 'offset': int(offset, 16),
                             'length': int(length, 16), 'module': module,
                             'acbp': int(acbp, 16)})
        elif not line.startswith(' Start'):
            match = SEGMENT_ROW.match(line)
            if match:
                start, stop, length, name, cls = match.groups()
                segments.append({'start': int(start, 16), 'stop': int(stop, 16),
                                 'length': int(length, 16), 'name': name, 'class': cls})
    return segments, rows


def linker_files(linker):
    names = ['TLINK.EXE', 'DPMI16BI.OVL', 'DPMILOAD.EXE', 'DPMIMEM.DLL']
    result = []
    for name in names:
        path = linker.parent / name
        if path.exists():
            result.append(path)
    return result


def run(root=ROOT, linker=DEFAULT_LINKER, dosbox=None):
    linker = Path(linker).resolve()
    if not linker.exists():
        raise ValueError(f'linker candidate is unavailable: {linker}')
    dosbox = Path(dosbox or os.environ.get(
        'DOSBOX', 'C:/Program Files/DOSBox Staging/dosbox.exe')).resolve()
    lock = read_json(root / 'layout/toolchain.json')
    manifest = read_json(root / 'layout/manifest.json')
    c0c = root / 'toolchain/C0C.OBJ'
    cc_lib = root / 'toolchain/CC.LIB'
    if not c0c.exists() or not cc_lib.exists():
        raise ValueError('run tools/setup_toolchain.py first to install C0C.OBJ and CC.LIB')
    startup_length = 0x1BC
    owners = [o for o in manifest['regions']
              if o['kind'] == 'MATCHING_C' and o['start'] >= 512 + startup_length]
    root_build = root / 'build'
    root_build.mkdir(exist_ok=True)
    work = Path(tempfile.mkdtemp(prefix='tlink-structural-', dir=root_build)).resolve()
    compile_work = work / 'compile'
    receipts, compile_session = compile_sources(root, owners, compile_work,
                                                root / 'toolchain', dosbox, lock)
    tc_lib = work / 'TC/LIB'
    bc_bin = work / 'BC/BIN'
    dos_work = work / 'WORK'
    tc_lib.mkdir(parents=True)
    bc_bin.mkdir(parents=True)
    dos_work.mkdir(parents=True)
    shutil.copyfile(c0c, tc_lib / 'C0C.OBJ')
    shutil.copyfile(cc_lib, tc_lib / 'CC.LIB')
    for path in linker_files(linker):
        shutil.copyfile(path, bc_bin / path.name.upper())
    scaffold = []
    object_names = []
    owner_by_id = {owner['id']: owner for owner in owners}
    code_end = max(owner['end'] for owner in owners)
    pad_index = 0
    for region in sorted(manifest['regions'], key=lambda item: item['start']):
        if region['id'] in owner_by_id:
            owner = owner_by_id[region['id']]
            receipt = receipts[owner['id']]
            source = compile_work / receipt['object']
            source_bytes = source.read_bytes()
            owned_length = owner['end'] - owner['start']
            trimmed = trim_text_contribution(source_bytes, owned_length)
            staged = dos_work / Path(receipt['object']).name
            staged.write_bytes(trimmed)
            object_names.append(staged.name)
            scaffold.append({
                'kind': 'owner',
                'owner': owner['id'],
                'object': staged.name,
                'original_sha256': sha(source_bytes),
                'staged_sha256': sha(trimmed),
                'original_size': len(source_bytes),
                'staged_size': len(trimmed),
                'owned_text_length': owned_length,
            })
        elif (region.get('classification') == 'alignment_padding'
              and startup_length + 512 <= region['start'] < code_end):
            length = region['end'] - region['start']
            name = f'P{pad_index:04d}.OBJ'
            pad_index += 1
            padding = make_text_padding(length, Path(name).stem)
            (dos_work / name).write_bytes(padding)
            object_names.append(name)
            scaffold.append({
                'kind': 'alignment_padding',
                'owner': region['id'],
                'object': name,
                'staged_sha256': sha(padding),
                'staged_size': len(padding),
                'owned_text_length': length,
            })
    response = '/s C:\\TC\\LIB\\C0C.OBJ+' + '+'.join(
        f'C:\\WORK\\{name}' for name in object_names)
    response += ',OUT.EXE,OUT.MAP,C:\\TC\\LIB\\CC.LIB'
    (work / 'LINK.RSP').write_text(response, encoding='ascii')
    batch = ('@echo off\r\n'
             'c:\r\n'
             'cd \\work\r\n'
             'set PATH=C:\\BC\\BIN\r\n'
             'tlink @C:\\LINK.RSP > LINK.LOG\r\n'
             'echo DONE>DONE.TXT\r\n')
    (work / 'GO.BAT').write_bytes(batch.encode('ascii'))
    config = ('[sdl]\noutput=texture\n[mixer]\nnosound=true\n[dosbox]\n'
              'machine=svga_s3\nmemsize=16\n[cpu]\ncore=auto\n'
              'cputype=auto\n[autoexec]\n'
              f'mount c "{work}"\nc:\ncall c:\\go.bat\nexit\n')
    config_path = work / 'dosbox.conf'
    config_path.write_text(config, encoding='utf-8')
    command = [str(dosbox), '-conf', str(config_path), '--noprimaryconfig',
               '-noconsole', '-exit']
    result = subprocess.run(command, cwd=work, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT, timeout=600)
    log_path = dos_work / 'LINK.LOG'
    map_path = dos_work / 'OUT.MAP'
    log = log_path.read_text(errors='replace') if log_path.exists() else ''
    unresolved = [line.strip() for line in log.splitlines()
                  if 'Undefined symbol' in line]
    segments, rows = parse_map(map_path) if map_path.exists() else ([], [])
    code_rows = [row for row in rows if row['module'].startswith('R')]
    divergences = []
    for index, owner in enumerate(owners):
        expected_start = owner['start'] - 512
        expected_length = owner['end'] - owner['start']
        if index >= len(code_rows):
            divergences.append({'index': index, 'owner': owner['id'],
                                'reason': 'missing_linker_row'})
            break
        row = code_rows[index]
        if row['offset'] != expected_start or row['length'] != expected_length:
            divergences.append({'index': index, 'owner': owner['id'],
                                'module': row['module'],
                                'expected_start': expected_start,
                                'actual_start': row['offset'],
                                'expected_length': expected_length,
                                'actual_length': row['length']})
            break
    report = {
        'format': 'empires-tlink-structural-experiment-v1',
        'status': 'MAP_AVAILABLE' if map_path.exists() else 'LINK_FAILED',
        'linker': {'path': str(linker), 'sha256': sha(linker.read_bytes()),
                   'files': [{'name': p.name, 'sha256': sha(p.read_bytes())}
                             for p in linker_files(linker)]},
        'startup_object': {'path': 'C0C.OBJ', 'sha256': sha(c0c.read_bytes()),
                           'text_bytes': startup_length},
        'compile': {'owner_count': len(owners), 'session': compile_session},
        'relocatable_scaffold': scaffold,
        'link': {'returncode': result.returncode, 'command': command,
                 'unresolved_symbols': unresolved[:200],
                 'unresolved_count': len(unresolved)},
        'segments': segments,
        'code_rows': rows,
        'code_comparison': {'expected_owner_count': len(owners),
                            'actual_code_row_count': len(code_rows),
                            'first_divergence': divergences[0] if divergences else None},
        'outputs': {
            'exe_sha256': sha((dos_work / 'OUT.EXE').read_bytes())
            if (dos_work / 'OUT.EXE').exists() else None,
            'map_sha256': sha(map_path.read_bytes()) if map_path.exists() else None,
        },
        'interpretation': 'TLINK placement is experimental; the fixed manifest remains the oracle and unresolved data symbols are expected until the synthetic DGROUP scaffold exists.'
    }
    write_json(root_build / 'tlink-structural-report.json', report)
    print(f"TLINK map: {report['status']}; code rows {len(code_rows)}/{len(owners)}; unresolved {len(unresolved)}")
    print(f"First code divergence: {report['code_comparison']['first_divergence']}")
    print(f"Report: {root_build / 'tlink-structural-report.json'}")
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--linker', type=Path, default=DEFAULT_LINKER)
    parser.add_argument('--dosbox', type=Path)
    args = parser.parse_args()
    try:
        run(linker=args.linker, dosbox=args.dosbox)
    except (OSError, ValueError, KeyError, subprocess.SubprocessError) as error:
        print(f'FAIL: {error}', file=sys.stderr)
        return 1
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
