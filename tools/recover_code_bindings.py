"""Replace proven entry-address declarations with owned public references."""
import copy
import os
from pathlib import Path
import tempfile

from mz import MZ
from reconstruct import ROOT, read_json, write_json, reconstruct, sha


def derive(manifest, mz):
    updated = copy.deepcopy(manifest)
    targets = {mz.load_offset(r['start']): r for r in updated['regions']
               if r['kind'] in ('MATCHING_C', 'MATCHING_ASM')}
    changes = []
    for caller in updated['regions']:
        for symbol, binding in caller.get('build', {}).get('bindings', {}).items():
            target = targets.get(binding.get('offset'))
            if (binding['coordinate'] != 'code_offset' or target is None
                    or target['build']['public'] != symbol):
                continue
            old = binding.pop('offset')
            binding.update(owner=target['id'], public=symbol, addend=0)
            changes.append({'caller': caller['id'], 'symbol': symbol, 'owner': target['id'],
                            'previous_code_offset': old})
    return updated, changes


def promote(root=ROOT):
    path = root / 'layout/manifest.json'
    manifest = read_json(path)
    original = (root / manifest['original']['path']).read_bytes()
    if sha(original) != manifest['original']['sha256']:
        raise ValueError('Original identity mismatch')
    updated, changes = derive(manifest, MZ.parse(original))
    if not changes:
        print('No additional owned entry bindings to derive')
        return
    (root / 'build').mkdir(exist_ok=True)
    work = Path(tempfile.mkdtemp(prefix='code-bindings-', dir=root / 'build'))
    candidate = work / 'manifest.json'
    write_json(candidate, updated)
    report = reconstruct(root, candidate, work, root / 'toolchain',
        Path(os.environ.get('DOSBOX', 'C:/Program Files/DOSBox Staging/dosbox.exe')))
    # Canonical ownership changes only after a fresh complete executable match.
    write_json(path, updated)
    write_json(root / 'docs/code-binding-evidence.json', {
        'status': 'EQUAL', 'reconstructed_sha256': report['reconstructed_sha256'],
        'bindings_promoted': len(changes), 'changes': changes,
        'scope': 'Owned selected entry publics; fixed placement remains; no historical linker claim'})
    for name in ('AEPROG.EXE', 'report.json', 'game-report.json'):
        (root / 'build' / name).unlink(missing_ok=True)
    print(f'Promoted {len(changes)} code bindings to component-owned entry publics')


if __name__ == '__main__':
    promote()
