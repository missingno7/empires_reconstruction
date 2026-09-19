"""Promote explicit local C/ASM recipes only after fresh complete extent/fixup checks."""
import argparse
from pathlib import Path
import tempfile

from mz import MZ
from exe_data import encode_data, TEXT_FORMAT, RECORDS_FORMAT, U16_TABLE_FORMAT, ZERO_PAD_FORMAT
from storage_evidence import verify_bindings
from promote_upstream import replace_raw_owners
from reconstruct import (ROOT, read_json, write_json, project_path, sha, compile_sources,
                         read_object, bind_region, mismatch, owned_library_modules, compiled_data)


def promote(recipe_path, root=ROOT):
    recipe = read_json(recipe_path)
    if recipe['format'] != 'empires-c-promotion-v1':
        raise ValueError('Unknown C/ASM promotion recipe')
    manifest_path = root / 'layout/manifest.json'
    manifest = read_json(manifest_path)
    original = project_path(root, manifest['original']['path']).read_bytes()
    if len(original) != manifest['original']['size'] or sha(original) != manifest['original']['sha256']:
        raise ValueError('Original identity mismatch')
    existing = {r['id']: r for r in manifest['regions']}
    candidates = []
    for owner in recipe['owners']:
        if owner['kind'] not in ('MATCHING_C', 'MATCHING_ASM', 'KNOWN_TOOLCHAIN_LIBRARY') and not (owner['kind'] == 'EXACT_DATA' and owner['build']['encoder'] in ('omf-segment-v1', TEXT_FORMAT, RECORDS_FORMAT, U16_TABLE_FORMAT, ZERO_PAD_FORMAT)):
            raise ValueError('Recipe may only promote matching C/ASM, pinned libraries, compiled data and identified text')
        if owner['id'] in existing:
            if owner != existing[owner['id']]:
                raise ValueError('Existing ownership differs from candidate recipe')
            continue
        if sha(original[owner['start']:owner['end']]) != owner['expected_sha256']:
            raise ValueError('Candidate extent identity mismatch')
        candidates.append(owner)
    if not candidates:
        print('No new C/ASM candidates to promote')
        return
    proposed = replace_raw_owners(manifest, candidates, original)
    verify_bindings(root, proposed, original)
    (root / 'build').mkdir(exist_ok=True)
    work = Path(tempfile.mkdtemp(prefix='c-promotion-', dir=root / 'build'))
    lock = read_json(root / 'layout/toolchain.json')
    modules = owned_library_modules(proposed['regions'], root / 'toolchain', lock)
    needed = {o['id'] if o['kind'] in ('MATCHING_C', 'MATCHING_ASM') else o['build'].get('code_owner') for o in candidates}
    sources = [o for o in proposed['regions'] if o['id'] in needed and o['kind'] in ('MATCHING_C', 'MATCHING_ASM')]
    receipts, session = compile_sources(root, sources, work, root / 'toolchain',
                                       Path(lock['dosbox_default']), lock)
    for owner in sources:
        modules[owner['id']] = read_object((work / receipts[owner['id']]['object']).read_bytes())
    results = []
    for owner in candidates:
        if owner['kind'] in ('MATCHING_C', 'MATCHING_ASM'):
            receipt = receipts[owner['id']]
            data, proof = bind_region(owner, modules[owner['id']], MZ.parse(original), proposed['frames'], proposed['regions'], modules)
        elif owner['kind'] == 'KNOWN_TOOLCHAIN_LIBRARY':
            data, proof = bind_region(owner, modules[owner['id']], MZ.parse(original), proposed['frames'], proposed['regions'], modules)
            receipt = {'source_sha256': owner['build']['module_sha256']}
        elif owner['build']['encoder'] == 'omf-segment-v1':
            receipt = receipts[owner['build']['code_owner']]
            data, proof = compiled_data(owner, proposed['regions'], modules, MZ.parse(original),
                                        proposed['frames'])
        else:
            source = project_path(root, owner['source'])
            data = encode_data(read_json(source), owner['build']['encoder'])
            mz = MZ.parse(original)
            if any(mz.load_offset(owner['start']) - 1 <= r['load_offset'] < mz.load_offset(owner['end']) for r in mz.relocations):
                raise ValueError('Text owner overlaps an MZ relocation')
            receipt = {'source_sha256': sha(source.read_bytes())}
            proof = {'load_relocations': []}
        mismatch(original[owner['start']:owner['end']], data, owner)
        results.append({'id': owner['id'], 'kind': owner['kind'], 'bytes': len(data), 'sha256': sha(data),
                        'source_sha256': receipt['source_sha256'],
                        'fixups': len(proof.get('fixups', [])), 'load_relocations': proof['load_relocations'],
                        'encoder': owner.get('build', {}).get('encoder'), 'status': 'EQUAL'})
    report = {'status': 'EQUAL', 'recipe_sha256': sha(recipe_path.read_bytes()),
              'original_sha256': sha(original), 'promoted_c_bytes': sum(r['bytes'] for r in results if r['kind'] == 'MATCHING_C'),
              'promoted_asm_bytes': sum(r['bytes'] for r in results if r['kind'] == 'MATCHING_ASM'),
              'promoted_library_bytes': sum(r['bytes'] for r in results if r['kind'] == 'KNOWN_TOOLCHAIN_LIBRARY'),
              'promoted_compiled_data_bytes': sum(r['bytes'] for r in results if r['encoder'] == 'omf-segment-v1'),
              'promoted_text_bytes': sum(r['bytes'] for r in results if r['encoder'] == TEXT_FORMAT),
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
    print(f"Promoted {len(sources)} matching C/ASM functions, {report['promoted_c_bytes']} C bytes, {report['promoted_asm_bytes']} ASM bytes, {report['promoted_library_bytes']} library bytes, {report['promoted_compiled_data_bytes']} compiled data bytes and {report['promoted_text_bytes']} text bytes")
    return report


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--recipe', type=Path, default=ROOT / 'recipes/c/matching-wave1.json')
    args = parser.parse_args()
    promote(args.recipe)
