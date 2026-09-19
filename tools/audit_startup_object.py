"""Compare the pinned compact-model startup OBJ with the EXE prefix.

The comparison deliberately ignores bytes occupied by the OBJ's own FIXUPP
records.  All other bytes must match the original load image at load offset
zero.  This establishes module extent and invariant instruction bytes without
pretending that unresolved main/exit/data symbols have already been linked.
"""
import argparse
from pathlib import Path
import sys

from reconstruct import ROOT, project_path, read_json, sha, write_json
from omf import OmfReader


def audit(root=ROOT):
    lock = read_json(root / 'layout/toolchain.json')
    startup = next((x for x in lock.get('objects', [])
                    if x.get('role') == 'compact_model_startup'), None)
    if startup is None:
        raise ValueError('toolchain lock has no compact_model_startup object')
    object_path = project_path(root, 'toolchain/' + startup['path'])
    if not object_path.exists():
        raise ValueError(f'missing local startup object: {object_path}')
    object_bytes = object_path.read_bytes()
    if sha(object_bytes) != startup['sha256']:
        raise ValueError('startup object hash differs from the pinned local input')
    module = OmfReader().read(object_bytes, object_path.name)
    original = project_path(root, read_json(root / 'layout/manifest.json')['original']['path']).read_bytes()
    segment = '_TEXT'
    declared = module.segment_length(segment)
    if declared is None:
        raise ValueError('startup object has no _TEXT SEGDEF')
    source = bytearray(module.segment_bytes(segment))
    if len(source) > declared:
        raise ValueError('startup _TEXT data exceeds its SEGDEF length')
    source.extend(bytes(declared - len(source)))
    target = original[512:512 + declared]
    if len(target) != declared:
        raise ValueError('original EXE is shorter than the startup extent')
    fixed = {position for fixup in module.fixups_in(segment)
             for position in range(fixup['offset'], fixup['offset'] + fixup['width'])}
    mismatches = [offset for offset, (left, right) in enumerate(zip(source, target))
                  if offset not in fixed and left != right]
    result = {
        'format': 'empires-startup-object-evidence-v1',
        'status': 'EQUAL' if not mismatches else 'DIFFERS',
        'object': startup['path'],
        'object_sha256': sha(object_bytes),
        'segment': segment,
        'declared_text_bytes': declared,
        'materialized_text_bytes': len(module.segment_bytes(segment)),
        'original_load_offset': 0,
        'fixups': len(module.fixups_in(segment)),
        'fixup_bytes_ignored': len(fixed),
        'non_fixup_mismatches': mismatches,
        'segment_definition': next(x for x in module.segment_defs if x['name'] == segment),
        'groups': module.groups,
        'publics': module.publics_in(segment),
        'interpretation': 'C0C.OBJ accounts for the complete first _TEXT segment extent; unresolved fixup values remain a linker task.'
    }
    write_json(root / 'docs/startup-linker-evidence.json', result)
    print(result)
    if mismatches:
        return 1
    return 0


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.parse_args()
    try:
        return audit()
    except (OSError, KeyError, ValueError) as error:
        print(f'FAIL: {error}', file=sys.stderr)
        return 1


if __name__ == '__main__':
    raise SystemExit(main())
