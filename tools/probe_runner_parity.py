"""Compare the MS-DOS Player and DOSBox execution hosts for the pinned tools.

This is a runner regression probe, not a replacement build path.  It compiles
two representative source units through both hosts and then runs the complete
fresh structural build twice.  The final linked EXEs must be byte-identical.
"""
import argparse
from pathlib import Path
import shutil

from build_exe import build
from dos_runner import resolve_runner
from mz import MZ
from omf import OmfReader
from omf_scaffold import _record, _records
from reconstruct import ROOT, compile_sources, read_json, write_json


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

    reports, artifacts = {}, {}
    for label, runner in (('msdos-player', msdos), ('dosbox', reference)):
        report = build(root=root, verify=verify, runner=runner)
        target = output / label / 'AEPROG.EXE'
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(root / 'build/AEPROG.EXE', target)
        blob = target.read_bytes()
        reports[label] = report
        artifacts[label] = {'path': str(target), 'sha256': report['sha256'],
                            'relocations': [(item['segment'], item['offset']) for item in MZ.parse(blob).relocations]}
    if artifacts['msdos-player']['sha256'] != artifacts['dosbox']['sha256']:
        raise ValueError('MS-DOS Player and DOSBox linked EXEs differ')
    if artifacts['msdos-player']['relocations'] != artifacts['dosbox']['relocations']:
        raise ValueError('MS-DOS Player and DOSBox relocation tables differ')
    report = {'status': 'EXACT', 'runners': {'msdos_player': msdos.receipt(), 'dosbox': reference.receipt()},
              'representative_objects': representatives, 'linked_artifacts': artifacts,
              'verification_performed': verify}
    write_json(output / 'report.json', report)
    return report


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--msdos-player', type=Path, required=True)
    parser.add_argument('--dosbox', type=Path)
    parser.add_argument('--no-verify', action='store_true')
    args = parser.parse_args()
    result = run(msdos_player=args.msdos_player, dosbox=args.dosbox, verify=not args.no_verify)
    print(f"Runner parity: {result['status']}")
