"""Explicit maintenance operation: import proven sources and regenerate ownership.

Not called by the build. Never writes upstream. Run in a clean working tree:
this intentionally replaces the canonical manifest and its raw partition.
"""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import re
import shutil

from mz import MZ

ROOT = Path(__file__).resolve().parents[1]


def sha(data):
    return hashlib.sha256(data).hexdigest()


def write_json(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2) + '\n', encoding='utf-8')


def import_layout(upstream, ids):
    original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
    mz = MZ.parse(original)
    base = upstream / 'controls/correspondence'
    profile_path = base / 'profile.tc20-tasm10-dosbox.json'
    profile = json.loads(profile_path.read_text())
    frames = profile['layout']['frames']
    defaults = {b['symbol']: b for b in profile['layout']['bindings']}
    sources, inventory = [], []
    for path in sorted(base.glob('*.json')):
        entry = json.loads(path.read_text())
        if 'extent' not in entry:
            continue
        inventory_row = {'id': entry['id'], 'kind': entry['kind'], 'verdict': entry['verdict'], 'extent': entry['extent']}
        inventory.append(inventory_row)
        if entry['kind'] not in ('MATCHING_C', 'MATCHING_ASM') or entry['verdict'] != 'EQUAL':
            continue
        if ids is not None and entry['id'] not in ids:
            continue
        record_path = base / 'records' / path.name
        record = json.loads(record_path.read_text())
        extent = entry['extent']
        start, length = extent['file_offset'], extent['length']
        if record['verdict'] != 'EQUAL' or record['comparison']['excluded']:
            raise ValueError(f"{entry['id']}: upstream proof is not fully compared")
        if record['extent']['image_sha256'] != sha(original):
            raise ValueError('Upstream proof uses a different EXE')
        if mz.file_offset(extent['address']) != start or sha(original[start:start+length]) != extent['sha256']:
            raise ValueError(f"{entry['id']}: upstream extent differs")
        source_path = base / entry['source']['path']
        source_bytes = source_path.read_bytes()
        normalized = source_bytes.replace(b'\r\n', b'\n').replace(b'\n', b'\r\n')
        if record['source']['sha256'] not in (sha(source_bytes), sha(normalized)):
            inventory_row['import_status'] = 'skipped: source changed since upstream proof'
            print(f"Skipping {entry['id']}: source changed since upstream proof")
            continue
        target = ('src/' if entry['kind'] == 'MATCHING_C' else 'asm/') + source_path.name
        (ROOT / target).parent.mkdir(exist_ok=True)
        shutil.copyfile(source_path, ROOT / target)
        declared = dict(defaults)
        declared.update({b['symbol']: b for b in entry.get('bindings', [])})
        bindings = {}
        module_segments = {}
        for used in record['comparison']['bindings_used']:
            symbol = used['symbol']
            if symbol == '_TEXT' or symbol == 'DGROUP':
                continue  # module code base and segment paragraphs are derived at build time
            if symbol in ('_DATA', '_BSS'):
                module_segments[symbol] = {'offset': used['address'], 'coordinate': 'DGROUP_offset', 'evidence': used['model_key']}
                continue
            if symbol in declared:
                binding = declared[symbol]
                if 'value' in binding:
                    raise ValueError(f'{symbol}: written-value binding requires explicit review')
                value = binding['address'] - frames.get(binding.get('frame'), 0)
                coordinate = 'DGROUP_offset' if binding.get('frame') == 'DGROUP' else 'code_offset'
            else:
                match = re.fullmatch(r'_?[gbwsa]([0-9a-f]{1,6})', symbol)
                function = re.fullmatch(r'_?f([0-9a-f]{1,6})', symbol)
                if not (match or function):
                    raise ValueError(f'{symbol}: no reusable declared binding')
                value = int((match or function).group(1), 16)
                coordinate = 'DGROUP_offset' if match else 'code_offset'
            bindings[symbol] = {'offset': value, 'coordinate': coordinate, 'evidence': used['model_key']}
        owner = {
            'id': entry['id'], 'start': start, 'end': start + length,
            'kind': entry['kind'], 'classification': 'code', 'source': target,
            'original_symbol': entry['id'], 'artifact': f"regions/{entry['id']}.bin",
            'expected_sha256': extent['sha256'], 'matching_status': 'EQUAL',
            'build': {'segment': entry['source']['segment'], 'public': entry['source']['public'],
                      'span': entry['source'].get('span', 1),
                      'flags_append': entry.get('build', {}).get('flags_append', ''),
                      'bindings': bindings, 'module_segments': module_segments},
            'provenance': {'project': 'empires_forged', 'entry': str(path.relative_to(upstream)).replace('\\', '/'),
                           'entry_sha256': sha(path.read_bytes()), 'record_sha256': sha(record_path.read_bytes()),
                           'source_sha256': sha(source_bytes), 'profile_sha256': sha(profile_path.read_bytes())}}
        sources.append(owner)
        inventory_row['import_status'] = 'imported'
    if ids is not None and set(ids) != {r['id'] for r in sources}:
        raise ValueError('Requested source IDs not all imported')
    regions = []
    def raw(start, end, classification='unknown'):
        if start == end:
            return
        name = f'raw/{start:06X}-{end:06X}.bin'
        data = original[start:end]
        (ROOT / name).parent.mkdir(exist_ok=True)
        (ROOT / name).write_bytes(data)
        regions.append({'id': f'RAW_{start:06X}', 'start': start, 'end': end, 'kind': 'RAW',
                        'classification': classification, 'source': name, 'original_symbol': None,
                        'artifact': name, 'expected_sha256': sha(data), 'matching_status': 'EQUAL'})
    raw(0, mz.header_size, 'MZ_header_and_relocations')
    cursor = mz.header_size
    for owner in sorted(sources, key=lambda r: r['start']):
        if owner['start'] < cursor:
            raise ValueError('Overlapping upstream source extents')
        raw(cursor, owner['start'])
        regions.append(owner)
        cursor = owner['end']
    raw(cursor, mz.declared_size)
    raw(mz.declared_size, len(original), 'trailing_file_bytes')
    manifest = {'format': 'empires-owned-exe-v1', 'range_convention': '[start,end) file offsets',
                'original': {'path': 'assets/AEPROG.EXE', 'size': len(original), 'sha256': sha(original)},
                'frames': {'_TEXT': 0, 'DGROUP': frames['DGROUP'] - frames['_TEXT']},
                'regions': regions}
    write_json(ROOT / 'layout/manifest.json', manifest)
    write_json(ROOT / 'docs/upstream-inventory.json', {
        'upstream': str(upstream), 'profile_sha256': sha(profile_path.read_bytes()),
        'census': dict(Counter(e['kind'] for e in inventory)), 'entries': inventory})
    print(f'Imported {len(sources)} source regions; {len(regions)} owners; {len(original)} bytes')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--upstream', type=Path, default=Path('D:/Games/DOS/dos_recosystem/empires_forged'))
    parser.add_argument('--ids', nargs='*', help='Only these source IDs; empty list makes raw-only baseline')
    args = parser.parse_args()
    import_layout(args.upstream, args.ids)
