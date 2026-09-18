"""Compile an ordered C source group into one OBJ and check every owned extent."""
import argparse
import os
from pathlib import Path
import subprocess
import sys
import tempfile

from mz import MZ
from reconstruct import ROOT, bind_region, compile_sources, mismatch, project_path, read_json, read_object, sha, write_json


def probe(recipe_path, root=ROOT, toolchain=None, dosbox=None):
    (root / 'build').mkdir(exist_ok=True)
    (root / 'build/module-group-report.json').unlink(missing_ok=True)
    recipe = read_json(recipe_path)
    if recipe['format'] != 'empires-c-module-candidate-v1':
        raise ValueError('Unknown module candidate recipe')
    work = Path(tempfile.mkdtemp(prefix='module-group-', dir=root / 'build')).resolve()
    combined = b'\r\n'.join(project_path(root, s['path']).read_bytes() for s in recipe['sources'])
    source = work / 'combined.C'
    source.write_bytes(combined)
    candidate = {'id': recipe['id'], 'kind': 'MATCHING_C', 'source': source.relative_to(root).as_posix(),
                 'build': {'flags_append': recipe['flags_append']}}
    # The compiler sees only the ordered source pieces and compiler settings.
    receipts, session = compile_sources(root, [candidate], work,
                                       toolchain or root / 'toolchain',
                                       dosbox or Path(os.environ.get('DOSBOX', 'C:/Program Files/DOSBox Staging/dosbox.exe')),
                                       read_json(root / 'layout/toolchain.json'))
    module = read_object((work / receipts[recipe['id']]['object']).read_bytes())
    # Original locations enter only the comparison/binding scaffold below.
    manifest = read_json(root / 'layout/manifest.json')
    original = project_path(root, manifest['original']['path']).read_bytes()
    if sha(original) != manifest['original']['sha256'] or len(original) != manifest['original']['size']:
        raise ValueError('Original EXE identity differs')
    owners = {o['id']: o for o in manifest['regions']}
    selected = [owners[s['owner']] for s in recipe['sources']]
    if not selected or any(a['end'] != b['start'] for a, b in zip(selected, selected[1:])):
        raise ValueError('Candidate owners must be contiguous and in source order')
    publics = module.publics_in(recipe['segment'])
    expected_names = [s['public'] for s in recipe['sources']]
    if [p['name'] for p in publics] != expected_names:
        raise ValueError('Combined module public order differs')
    if module.segment_length(recipe['segment']) != selected[-1]['end'] - selected[0]['start']:
        raise ValueError('Combined module emitted extent size differs')
    mz, results, bases, all_fixups = MZ.parse(original), [], set(), []
    for spec, owner, public in zip(recipe['sources'], selected, publics):
        if owner['source'] != spec['path'] or owner['build']['flags_append'] != recipe['flags_append']:
            raise ValueError('Recipe differs from established source/flags')
        expected_offset = owner['start'] - selected[0]['start']
        if public['offset'] != expected_offset:
            raise ValueError(f"{owner['id']}: emitted module-relative public offset differs")
        data, proof = bind_region(owner, module, mz, manifest['frames'])
        mismatch(original[owner['start']:owner['end']], data, owner)
        bases.add(proof['module_load_base'])
        all_fixups.extend(proof['fixups'])
        results.append({'owner': owner['id'], 'bytes': len(data), 'public_offset': public['offset'],
                        'source_sha256': sha(project_path(root, spec['path']).read_bytes()), 'status': 'EQUAL', **proof})
    if len(bases) != 1:
        raise ValueError('Individual extents imply inconsistent module placement')
    report = {'status': 'EQUAL', 'candidate': recipe['id'], 'historical_module_proven': False,
              'compile': receipts[recipe['id']], 'source_recipe_sha256': sha(recipe_path.read_bytes()),
              'text_bytes': module.segment_length(recipe['segment']), 'source_units_combined': len(selected),
              'fixups_checked': len(all_fixups), 'module_load_base_in_comparison_scaffold': bases.pop(),
              'publics': publics, 'owners': results, 'session': session,
              'limitation': 'Proves compatible shared compilation and emitted relative layout, not historical module boundaries, data ownership or linker reconstruction.'}
    write_json(work / 'report.json', report)
    write_json(root / 'build/module-group-report.json', report)
    print(f"{recipe['id']}: one fresh OBJ; {len(selected)} functions, {report['text_bytes']} bytes and {len(all_fixups)} fixups EQUAL")
    return report


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--recipe', type=Path, default=ROOT / 'recipes/modules/C_6C26_6C87.json')
    args = parser.parse_args()
    try:
        probe(args.recipe)
    except (ValueError, OSError, KeyError, subprocess.SubprocessError) as error:
        print(f'FAIL: {error}', file=sys.stderr)
        sys.exit(1)
