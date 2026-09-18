"""Promote explicit local C recipes only after fresh complete extent/fixup checks."""
import argparse
from pathlib import Path
import tempfile

from mz import MZ
from promote_upstream import replace_raw_owners
from reconstruct import (ROOT, read_json, write_json, project_path, sha, compile_sources,
                         read_object, bind_region, mismatch, owned_library_modules)


def promote(recipe_path, root=ROOT):
    recipe = read_json(recipe_path)
    if recipe['format'] != 'empires-c-promotion-v1':
        raise ValueError('Unknown C promotion recipe')
    manifest_path = root / 'layout/manifest.json'
    manifest = read_json(manifest_path)
    original = project_path(root, manifest['original']['path']).read_bytes()
    if len(original) != manifest['original']['size'] or sha(original) != manifest['original']['sha256']:
        raise ValueError('Original identity mismatch')
    existing = {r['id']: r for r in manifest['regions']}
    candidates = []
    for owner in recipe['owners']:
        if owner['kind'] != 'MATCHING_C':
            raise ValueError('Recipe may only promote matching C candidates')
        if owner['id'] in existing:
            if owner != existing[owner['id']]:
                raise ValueError('Existing ownership differs from candidate recipe')
            continue
        if sha(original[owner['start']:owner['end']]) != owner['expected_sha256']:
            raise ValueError('Candidate extent identity mismatch')
        candidates.append(owner)
    if not candidates:
        print('No new C candidates to promote')
        return
    proposed = replace_raw_owners(manifest, candidates, original)
    (root / 'build').mkdir(exist_ok=True)
    work = Path(tempfile.mkdtemp(prefix='c-promotion-', dir=root / 'build'))
    lock = read_json(root / 'layout/toolchain.json')
    modules = owned_library_modules(proposed['regions'], root / 'toolchain', lock)
    receipts, session = compile_sources(root, candidates, work, root / 'toolchain',
                                       Path(lock['dosbox_default']), lock)
    results = []
    for owner in candidates:
        module = read_object((work / receipts[owner['id']]['object']).read_bytes())
        data, proof = bind_region(owner, module, MZ.parse(original), proposed['frames'], proposed['regions'], modules)
        mismatch(original[owner['start']:owner['end']], data, owner)
        results.append({'id': owner['id'], 'bytes': len(data), 'sha256': sha(data),
                        'source_sha256': receipts[owner['id']]['source_sha256'],
                        'fixups': len(proof['fixups']), 'load_relocations': proof['load_relocations'],
                        'status': 'EQUAL'})
    report = {'status': 'EQUAL', 'recipe_sha256': sha(recipe_path.read_bytes()),
              'original_sha256': sha(original), 'promoted_c_bytes': sum(r['bytes'] for r in results),
              'owners': results}
    write_json(work / 'proof.json', {**report, 'compile': receipts, 'session': session})
    # No canonical ownership changes until every proposed C extent has passed.
    for name in ('AEPROG.EXE', 'report.json', 'game-report.json'):
        (root / 'build' / name).unlink(missing_ok=True)
    for owner in proposed['regions']:
        if owner['kind'] == 'RAW':
            target = project_path(root, owner['source'])
            if not target.exists():
                target.parent.mkdir(parents=True, exist_ok=True)
                target.write_bytes(original[owner['start']:owner['end']])
    write_json(manifest_path, proposed)
    evidence_name = 'c-matching-evidence.json' if recipe_path.stem == 'matching-wave1' else recipe_path.stem + '-evidence.json'
    write_json(root / 'docs' / evidence_name, report)
    print(f"Promoted {len(results)} matching C functions, {report['promoted_c_bytes']} bytes")
    return report


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--recipe', type=Path, default=ROOT / 'recipes/c/matching-wave1.json')
    args = parser.parse_args()
    promote(args.recipe)
