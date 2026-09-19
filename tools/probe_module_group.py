"""Compile an ordered C source group into one OBJ and check every owned extent."""
import argparse
import copy
import os
from pathlib import Path
import subprocess
import sys
import tempfile

from mz import MZ, encode_header
from reconstruct import ROOT, bind_region, compile_sources, mismatch, project_path, read_json, read_object, sha, write_json
from reconstruct import owned_library_modules


def probe(recipe_path, root=ROOT, toolchain=None, dosbox=None, verify=True):
    (root / 'build').mkdir(exist_ok=True)
    (root / 'build/module-group-report.json').unlink(missing_ok=True)
    recipe = read_json(recipe_path)
    if recipe['format'] != 'empires-c-module-candidate-v1':
        raise ValueError('Unknown module candidate recipe')
    work = Path(tempfile.mkdtemp(prefix='module-group-', dir=root / 'build')).resolve()
    prelude = recipe.get('prelude', '')
    if not isinstance(prelude, str) or not prelude.isascii():
        raise ValueError('Module candidate prelude must be ASCII text')
    pieces = [prelude.encode('ascii')] if prelude else []
    pieces.extend(project_path(root, s['path']).read_bytes() for s in recipe['sources'])
    combined = b'\r\n'.join(pieces)
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
    manifest = read_json(root / 'layout/manifest.json')
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
    header = read_json(root / 'layout/mz-header.json')
    mz = MZ.parse_header(encode_header(header, manifest['original']['size']), manifest['original']['size'])
    original = None
    if verify:
        original = project_path(root, manifest['original']['path']).read_bytes()
        if sha(original) != manifest['original']['sha256'] or len(original) != manifest['original']['size']:
            raise ValueError('Original EXE identity differs')
    results, bases, all_fixups = [], set(), []
    data_evidence = None
    data_owners = [o for o in manifest['regions']
                   if o.get('build', {}).get('encoder') == 'omf-segment-v1'
                   and o['build']['code_owner'] in {s['id'] for s in selected}]
    data_owners.sort(key=lambda o: o['start'])
    if module.segment_length('_DATA'):
        if (not data_owners or
                any(a['end'] != b['start'] for a, b in zip(data_owners, data_owners[1:]))):
            raise ValueError('Shared DATA requires complete contiguous canonical ownership')
        if module.fixups_in('_DATA'):
            raise ValueError('Shared DATA pointer fixups require a separate binding proof')
        expected_data_size = data_owners[-1]['end'] - data_owners[0]['start']
        expected_data_sha = data_owners[0]['expected_sha256'] if len(data_owners) == 1 else None
        if len(module.segment_bytes('_DATA')) != expected_data_size:
            raise ValueError('Shared DATA length differs from canonical ownership')
        if expected_data_sha and sha(module.segment_bytes('_DATA')) != expected_data_sha:
            raise ValueError('Shared DATA digest differs from canonical ownership')
        if verify and module.segment_bytes('_DATA') != original[data_owners[0]['start']:data_owners[-1]['end']]:
            raise ValueError('Shared DATA bytes/order differ from canonical ownership')
        data_evidence = {'bytes': expected_data_size, 'sha256': sha(module.segment_bytes('_DATA')),
                         'owners': [o['id'] for o in data_owners],
                         'fixups': 0, 'status': 'EQUAL'}
    component_modules = owned_library_modules(manifest['regions'], toolchain or root / 'toolchain', read_json(root / 'layout/toolchain.json'))
    for spec, owner, public in zip(recipe['sources'], selected, publics):
        if owner['source'] != spec['path'] or owner['build']['flags_append'] != recipe['flags_append']:
            raise ValueError('Recipe differs from established source/flags')
        expected_offset = owner['start'] - selected[0]['start']
        if public['offset'] != expected_offset:
            raise ValueError(f"{owner['id']}: emitted module-relative public offset differs")
        if data_evidence and '_DATA' in owner['build'].get('module_segments', {}):
            # Fresh shared compilation supplies the initializer addends. Only
            # the comparison scaffold uses the verified common DATA base.
            owner = copy.deepcopy(owner)
            owner['build']['module_segments']['_DATA'] = {
                'owner': data_owners[0]['id'], 'coordinate': 'DGROUP_offset', 'addend': 0}
        data, proof = bind_region(owner, module, mz, manifest['frames'], manifest['regions'], component_modules)
        if verify:
            mismatch(original[owner['start']:owner['end']], data, owner)
        elif sha(data) != owner['expected_sha256']:
            raise ValueError(f"{owner['id']}: source-generated extent digest differs from canonical metadata")
        bases.add(proof['module_load_base'])
        all_fixups.extend(proof['fixups'])
        results.append({'owner': owner['id'], 'bytes': len(data), 'public_offset': public['offset'],
                        'source_sha256': sha(project_path(root, spec['path']).read_bytes()), 'status': 'EQUAL', **proof})
    if len(bases) != 1:
        raise ValueError('Individual extents imply inconsistent module placement')
    report = {'status': 'EQUAL', 'candidate': recipe['id'], 'historical_module_proven': False,
              'object_path': str(work / receipts[recipe['id']]['object']),
              'compile': receipts[recipe['id']], 'source_recipe_sha256': sha(recipe_path.read_bytes()),
              'text_bytes': module.segment_length(recipe['segment']), 'source_units_combined': len(selected),
              'fixups_checked': len(all_fixups), 'module_load_base_in_comparison_scaffold': bases.pop(),
              'publics': publics, 'owners': results, 'session': session,
              'data_evidence': data_evidence,
              'bss_bytes': module.segment_length('_BSS') or 0,
              'verification_fixture_used': verify,
              'limitation': 'Proves compatible shared compilation, checked DATA and emitted relative layout, not historical module boundaries or linker reconstruction.'}
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
