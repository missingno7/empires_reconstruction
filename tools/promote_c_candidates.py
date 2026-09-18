"""Promote explicit local C recipes only after fresh complete extent/fixup checks."""
import argparse
from pathlib import Path
import tempfile

from mz import MZ
from promote_upstream import replace_raw_owners
from reconstruct import (ROOT, read_json, write_json, project_path, sha, compile_sources,
                         read_object, bind_region, mismatch, owned_library_modules, compiled_data)


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
        if owner['kind'] != 'MATCHING_C' and not (owner['kind'] == 'EXACT_DATA' and owner['build']['encoder'] == 'omf-segment-v1'):
            raise ValueError('Recipe may only promote matching C and its compiled data')
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
    needed = {o['id'] if o['kind'] == 'MATCHING_C' else o['build']['code_owner'] for o in candidates}
    sources = [o for o in proposed['regions'] if o['id'] in needed and o['kind'] == 'MATCHING_C']
    receipts, session = compile_sources(root, sources, work, root / 'toolchain',
                                       Path(lock['dosbox_default']), lock)
    for owner in sources:
        modules[owner['id']] = read_object((work / receipts[owner['id']]['object']).read_bytes())
    results = []
    for owner in candidates:
        if owner['kind'] == 'MATCHING_C':
            receipt = receipts[owner['id']]
            data, proof = bind_region(owner, modules[owner['id']], MZ.parse(original), proposed['frames'], proposed['regions'], modules)
        else:
            receipt = receipts[owner['build']['code_owner']]
            data, proof = compiled_data(owner, proposed['regions'], modules, MZ.parse(original))
        mismatch(original[owner['start']:owner['end']], data, owner)
        results.append({'id': owner['id'], 'kind': owner['kind'], 'bytes': len(data), 'sha256': sha(data),
                        'source_sha256': receipt['source_sha256'],
                        'fixups': len(proof.get('fixups', [])), 'load_relocations': proof['load_relocations'],
                        'status': 'EQUAL'})
    report = {'status': 'EQUAL', 'recipe_sha256': sha(recipe_path.read_bytes()),
              'original_sha256': sha(original), 'promoted_c_bytes': sum(r['bytes'] for r in results if r['kind'] == 'MATCHING_C'),
              'promoted_compiled_data_bytes': sum(r['bytes'] for r in results if r['kind'] == 'EXACT_DATA'),
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
    print(f"Promoted {len(sources)} matching C functions, {report['promoted_c_bytes']} code bytes and {report['promoted_compiled_data_bytes']} compiled data bytes")
    return report


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--recipe', type=Path, default=ROOT / 'recipes/c/matching-wave1.json')
    args = parser.parse_args()
    promote(args.recipe)
