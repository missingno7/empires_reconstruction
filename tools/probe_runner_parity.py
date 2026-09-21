"""Compare the MS-DOS Player and DOSBox execution hosts for the pinned tools.

This is a runner regression probe, not a replacement build path.  It compiles
two representative source units through both hosts and then runs the complete
fresh structural build twice.  The final linked EXEs must be byte-identical,
every staged DOS input must be byte-identical, every compiler/assembler object
must be equivalent linker-visible OMF, and the link maps must agree.
"""
import argparse
from pathlib import Path
import shutil
import time

from build_exe import build
from dos_runner import resolve_runner
from mz import MZ
from omf import OmfReader
from omf_scaffold import _record, _records
from reconstruct import ROOT, compile_sources, read_json, sha, write_json


REPRESENTATIVES = ('F_01BC', 'RUNTIME_BLOCK')


def semantic_omf(path):
    module = OmfReader().read(path.read_bytes(), path.name)
    return {
        'segment_lengths': module.segment_lengths,
        'segment_bytes': {name: bytes(data).hex() for name, data in module.segments.items()},
        'publics': sorted(module.publics, key=lambda item: (item['segment'], item['offset'], item['name'])),
        'externals': sorted(module.externals),
        'fixups': sorted(module.fixups, key=lambda item: (item['segment'], item['offset'], item['width'], item['target'])),
        'segment_defs': module.segment_defs,
        'groups': module.groups,
    }


def normalize_volatile_omf_comments(data):
    """Erase only TASM's source timestamp bytes from a comparison copy."""
    records = []
    for kind, body in _records(data):
        if kind == OmfReader.COMENT and len(body) >= 6 and body[:2] == b'@\xe9':
            body = body[:2] + b'\0\0\0\0' + body[6:]
        records.append(_record(kind, body))
    return b''.join(records)


def compile_representative(root, lock, runner, owner_id, work):
    manifest = read_json(root / 'layout/manifest.json')
    owner = next(item for item in manifest['regions'] if item['id'] == owner_id)
    receipts, _ = compile_sources(root, [owner], work, root / 'toolchain', runner, lock)
    return work / receipts[owner_id]['object']


def session_artifacts(session):
    """Staged DOS inputs, compiler objects and the link map of one build session."""
    inputs, objects = {}, {}
    for folder in ('compile/WORK', 'bss/WORK', 'WORK'):
        for path in sorted((session / folder).glob('*')):
            name = f'{folder}/{path.name.upper()}'  # DOS names are case-insensitive
            suffix = path.suffix.upper()
            if suffix in ('.C', '.H', '.ASM'):
                inputs[name] = sha(path.read_bytes())
            elif suffix == '.OBJ' and folder != 'WORK':
                objects[name] = path.read_bytes()
    for path in sorted(session.glob('*.RSP')):
        # The response file names the same objects; only the host-specific drive
        # prefixes differ (C:\WORK\ under DOSBox, relative under MS-DOS Player).
        text = path.read_bytes().replace(b'C:\\WORK\\', b'').replace(b'C:\\TC\\LIB\\', b'..\\TC\\LIB\\')
        inputs[path.name] = sha(text)
    map_path = session / 'WORK/OUT.MAP'
    return {'inputs': inputs, 'objects': objects,
            'map': map_path.read_bytes() if map_path.exists() else b''}


def equivalent_omf(a, b, name):
    module_a, module_b = OmfReader().read(a, name), OmfReader().read(b, name)
    return (module_a.segments == module_b.segments
            and sorted(module_a.publics, key=str) == sorted(module_b.publics, key=str)
            and sorted(module_a.externals) == sorted(module_b.externals)
            and sorted(module_a.fixups, key=str) == sorted(module_b.fixups, key=str))


def compare_sessions(player, dosbox):
    inputs = sorted(k for k in set(player['inputs']) | set(dosbox['inputs'])
                    if player['inputs'].get(k) != dosbox['inputs'].get(k))
    objects, raw_identical, normalized_only = [], 0, []
    for name in sorted(set(player['objects']) | set(dosbox['objects'])):
        a, b = player['objects'].get(name), dosbox['objects'].get(name)
        if a is None or b is None:
            objects.append(name)
        elif a == b:
            raw_identical += 1
        elif normalize_volatile_omf_comments(a) == normalize_volatile_omf_comments(b) or equivalent_omf(a, b, name):
            normalized_only.append(name)
        else:
            objects.append(name)
    return {'inputs': inputs, 'objects': objects, 'raw_identical': raw_identical,
            'normalized_only': normalized_only, 'map': player['map'] != dosbox['map']}


def run(root=ROOT, msdos_player=None, dosbox=None, verify=True):
    lock = read_json(root / 'layout/toolchain.json')
    msdos = resolve_runner(lock, backend='msdos-player', executable=msdos_player)
    reference = resolve_runner(lock, backend='dosbox', executable=dosbox)
    output = root / 'build/runner-parity'
    shutil.rmtree(output, ignore_errors=True)
    output.mkdir(parents=True)

    representatives = {}
    for owner_id in REPRESENTATIVES:
        player_path = compile_representative(root, lock, msdos, owner_id, output / 'msdos-player' / owner_id)
        dosbox_path = compile_representative(root, lock, reference, owner_id, output / 'dosbox' / owner_id)
        player_bytes, dosbox_bytes = player_path.read_bytes(), dosbox_path.read_bytes()
        semantic_equal = semantic_omf(player_path) == semantic_omf(dosbox_path)
        raw_equal = player_bytes == dosbox_bytes
        normalized_equal = (normalize_volatile_omf_comments(player_bytes) ==
                            normalize_volatile_omf_comments(dosbox_bytes))
        if not semantic_equal:
            raise ValueError(f'{owner_id}: runner OMF semantics differ')
        representatives[owner_id] = {
            'msdos_player_obj': str(player_path), 'dosbox_obj': str(dosbox_path),
            'byte_identical': raw_equal, 'normalized_byte_identical': normalized_equal,
            'semantic_equal': semantic_equal,
            'note': (None if raw_equal else
                     'Equivalent linker-visible OMF; host-specific timestamp, PUBDEF and/or LEDATA record ordering differs.'),
        }

    reports, artifacts, sessions, timings = {}, {}, {}, {}
    for label, runner in (('msdos-player', msdos), ('dosbox', reference)):
        started = time.time()
        report = build(root=root, verify=verify, runner=runner)
        timings[label] = round(time.time() - started, 1)
        target = output / label / 'AEPROG.EXE'
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(root / 'build/AEPROG.EXE', target)
        blob = target.read_bytes()
        reports[label] = report
        artifacts[label] = {'path': str(target), 'sha256': report['sha256'],
                            'relocations': [(item['segment'], item['offset']) for item in MZ.parse(blob).relocations]}
        sessions[label] = session_artifacts(Path(report['session']))
    if artifacts['msdos-player']['sha256'] != artifacts['dosbox']['sha256']:
        raise ValueError('MS-DOS Player and DOSBox linked EXEs differ')
    if artifacts['msdos-player']['relocations'] != artifacts['dosbox']['relocations']:
        raise ValueError('MS-DOS Player and DOSBox relocation tables differ')
    differences = compare_sessions(sessions['msdos-player'], sessions['dosbox'])
    if differences['inputs'] or differences['objects'] or differences['map']:
        raise ValueError(f'Session artifacts differ between hosts: {differences}')
    report = {'status': 'EXACT', 'runners': {'msdos_player': msdos.receipt(), 'dosbox': reference.receipt()},
              'representative_objects': representatives, 'linked_artifacts': artifacts,
              'session_comparison': {'staged_inputs': len(sessions['dosbox']['inputs']),
                                     'objects': len(sessions['dosbox']['objects']),
                                     'raw_identical_objects': differences['raw_identical'],
                                     'normalized_only_objects': differences['normalized_only'],
                                     'map_equal': not differences['map']},
              'build_seconds': timings, 'verification_performed': verify}
    write_json(output / 'report.json', report)
    return report


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--msdos-player', type=Path)
    parser.add_argument('--dosbox', type=Path)
    parser.add_argument('--no-verify', action='store_true')
    args = parser.parse_args()
    result = run(msdos_player=args.msdos_player, dosbox=args.dosbox, verify=not args.no_verify)
    print(f"Runner parity: {result['status']} build seconds {result['build_seconds']} "
          f"objects raw-identical {result['session_comparison']['raw_identical_objects']} "
          f"normalized-only {len(result['session_comparison']['normalized_only_objects'])}")
