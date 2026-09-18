"""Promote exact standalone bitmap sources to editable logical PNG plus JSON."""
import copy
import sys

from archive_recipe import update_existing_recipe
from bitmap_sources import export_source, encode_source
from reconstruct import ROOT, read_json, write_json, project_path, sha
from resource_codecs import encode_payload


def promote(root=ROOT):
    name = 'AE000'
    path = root / f'layout/archives/{name}.json'
    manifest = read_json(path)
    original = project_path(root, manifest['original']['path']).read_bytes()
    if len(original) != manifest['original']['size'] or sha(original) != manifest['original']['sha256']:
        raise ValueError('Original archive identity differs')
    changed, plans, rows = copy.deepcopy(manifest), [], []
    work = root / 'build/png-promotion'
    work.mkdir(parents=True, exist_ok=True)
    for entry in changed['resources']:
        if entry['kind'] != 'MATCHING_RESOURCE' or entry.get('source_format') != 'bitmap4-json-v1':
            continue
        bitmap = read_json(project_path(root, entry['source']))
        candidate = work / f'{entry["index"]:03d}.json'
        document = export_source(bitmap, candidate)
        payload = encode_source(document, candidate)
        if len(payload) != entry['decoded']['size'] or sha(payload) != entry['decoded']['sha256']:
            raise ValueError(f"{entry['id']}: PNG reconstruction differs from decoded identity")
        block = bytes((entry['rtype'], entry['flags'])) + encode_payload(payload, entry['flags'])
        if block != original[entry['start']:entry['end']] or sha(block) != entry['expected_sha256']:
            raise ValueError(f"{entry['id']}: PNG source does not reproduce complete original resource")
        entry['source_format'] = 'bitmap4-png-v1'
        entry['source'] = f'raw/{name}/png/{entry["index"]:03d}.json'
        plans.append((project_path(root, entry['source']), candidate.read_bytes(), candidate.with_suffix('.png').read_bytes()))
        rows.append({'id': entry['id'], 'payload_bytes': len(payload), 'payload_sha256': sha(payload),
                     'resource_bytes': len(block), 'resource_sha256': sha(block), 'status': 'EQUAL'})
    if plans:
        for filename in ('game-report.json', 'archives-report.json', 'AE000.DAT', 'AE001.DAT'):
            (root / 'build' / filename).unlink(missing_ok=True)
        for target, metadata, png in plans:
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(metadata)
            target.with_suffix('.png').write_bytes(png)
        temporary = path.with_suffix('.json.tmp')
        write_json(temporary, changed)
        temporary.replace(path)
        update_existing_recipe(root, name, changed)
    write_json(work / 'report.json', {'promoted': len(rows), 'resources': rows})
    print(f'Promoted {len(rows)} exact resources to PNG plus JSON; original pixel indices and display tables preserved')
    return rows


if __name__ == '__main__':
    try:
        promote()
    except (ValueError, OSError, KeyError) as error:
        print(f'FAIL: {error}', file=sys.stderr)
        sys.exit(1)
