"""Find minimal shared-compilation candidates from descending relocation runs."""
from pathlib import Path
from mz import MZ
from reconstruct import ROOT, read_json, sha, write_json


def candidates(manifest, relocations):
    runs, current = [], []
    for relocation in relocations:
        location = relocation['load_offset']
        owner = next(o for o in manifest['regions'] if o['start'] <= location + 512 < o['end'])
        if owner['kind'] != 'MATCHING_C' or current and location >= current[-1]['load_offset']:
            if current:
                runs.append(current)
                current = []
        if owner['kind'] == 'MATCHING_C':
            current.append({'load_offset': location, 'owner': owner['id']})
    if current:
        runs.append(current)
    result = []
    for run in runs:
        if len({r['owner'] for r in run}) < 2:
            continue
        low, high = min(r['load_offset'] for r in run), max(r['load_offset'] for r in run)
        owners = [o for o in manifest['regions'] if o['start'] <= high + 512 and o['end'] > low + 512]
        compatible = (all(o['kind'] == 'MATCHING_C' for o in owners) and
                      len({o['build']['flags_append'] for o in owners}) == 1)
        result.append({'owners': [o['id'] for o in owners], 'relocation_run': run,
                       'compatible_flags': compatible,
                       'constraint': 'minimal interval containing descending cross-owner run; boundaries may extend'})
    return result


def run():
    manifest = read_json(ROOT / 'layout/manifest.json')
    original = (ROOT / manifest['original']['path']).read_bytes()
    if sha(original) != manifest['original']['sha256']:
        raise ValueError('Oracle identity differs')
    groups = candidates(manifest, MZ.parse(original).relocations)
    by_id = {o['id']: o for o in manifest['regions']}
    for group in groups:
        if not group['compatible_flags']:
            continue
        selected = [by_id[owner] for owner in group['owners']]
        recipe_id = 'RELOC_' + selected[0]['id'] + '_' + selected[-1]['id']
        path = ROOT / 'build/relocation-group-recipes' / (recipe_id + '.json')
        path.parent.mkdir(parents=True, exist_ok=True)
        write_json(path, {'format': 'empires-c-module-candidate-v1', 'id': recipe_id,
                         'segment': '_TEXT', 'flags_append': selected[0]['build']['flags_append'],
                         'sources': [{'owner': o['id'], 'path': o['source'], 'public': o['build']['public']}
                                     for o in selected]})
        group['recipe_id'] = recipe_id
    report = {'format': 'empires-relocation-group-constraints-v1', 'groups': groups,
              'historical_modules_proven': 0,
              'limitation': 'Relocation-order constraints; candidates require fresh compilation and linker checks'}
    write_json(ROOT / 'docs/relocation-group-constraints.json', report)
    print(f'{len(groups)} cross-owner relocation grouping constraints')
    return report


if __name__ == '__main__':
    run()
