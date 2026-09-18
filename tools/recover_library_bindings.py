"""Replace established library addresses with freshly read OMF public references."""
import copy
import os
from pathlib import Path
import tempfile

from mz import MZ
from reconstruct import (ROOT, read_json, write_json, reconstruct, sha,
                         owned_library_modules, project_path)


def derive(manifest, mz, modules):
    updated = copy.deepcopy(manifest)
    publics = {}
    for owner in updated['regions']:
        if owner['kind'] != 'KNOWN_TOOLCHAIN_LIBRARY':
            continue
        for public in modules[owner['id']].publics_in(owner['build']['segment']):
            publics.setdefault(public['name'], []).append((owner, public['offset']))
    changes = []
    for caller in updated['regions']:
        for symbol, binding in caller.get('build', {}).get('bindings', {}).items():
            if binding['coordinate'] != 'code_offset' or 'offset' not in binding:
                continue
            targets = [(owner, offset) for owner, offset in publics.get(symbol, [])
                       if 0 <= offset < owner['end'] - owner['start']
                       and mz.load_offset(owner['start']) + offset == binding['offset']]
            if len(targets) != 1:
                continue
            target, relative = targets[0]
            old = binding.pop('offset')
            binding.update(owner=target['id'], public=symbol, addend=0)
            changes.append({'caller': caller['id'], 'symbol': symbol, 'owner': target['id'],
                            'previous_code_offset': old, 'omf_public_offset': relative})
    return updated, changes


def promote(root=ROOT):
    path = root / 'layout/manifest.json'
    manifest = read_json(path)
    original = project_path(root, manifest['original']['path']).read_bytes()
    if sha(original) != manifest['original']['sha256']:
        raise ValueError('Original identity mismatch')
    modules = owned_library_modules(manifest['regions'], root / 'toolchain', read_json(root / 'layout/toolchain.json'))
    updated, changes = derive(manifest, MZ.parse(original), modules)
    if not changes:
        print('No additional owned library bindings to derive')
        return
    (root / 'build').mkdir(exist_ok=True)
    work = Path(tempfile.mkdtemp(prefix='library-bindings-', dir=root / 'build'))
    candidate = work / 'manifest.json'
    write_json(candidate, updated)
    report = reconstruct(root, candidate, work, root / 'toolchain',
        Path(os.environ.get('DOSBOX', 'C:/Program Files/DOSBox Staging/dosbox.exe')))
    write_json(path, updated)
    write_json(root / 'docs/library-binding-evidence.json', {
        'status': 'EQUAL', 'reconstructed_sha256': report['reconstructed_sha256'],
        'bindings_promoted': len(changes), 'changes': changes,
        'scope': 'Pinned OMF public offsets plus fixed module placement; not historical linking'})
    for name in ('AEPROG.EXE', 'report.json', 'game-report.json'):
        (root / 'build' / name).unlink(missing_ok=True)
    print(f'Promoted {len(changes)} bindings to owned OMF library publics')


if __name__ == '__main__':
    promote()
