"""Prove bitmap and level payload round trips; promote only whole-resource matches."""
import argparse
import copy
from archive_recipe import update_existing_recipe

from dat_archive import assemble_archive
from reconstruct import ROOT, project_path, read_json, sha, write_json
from reconstruct_archives import NAMES
from resource_codecs import decode_payload, encode_payload
from resource_formats import bitmap_document, encode_bitmap, level_document, encode_level


def probe(root=ROOT, promote=False):
    rows, plans = [], []
    output = root / 'build/resource-structures'
    output.mkdir(parents=True, exist_ok=True)
    for name in NAMES:
        path = root / f'layout/archives/{name}.json'
        manifest = read_json(path)
        changed = copy.deepcopy(manifest)
        original = project_path(root, manifest['original']['path']).read_bytes()
        if len(original) != manifest['original']['size'] or sha(original) != manifest['original']['sha256']:
            raise ValueError(f'{name}: original identity differs')
        payloads, outputs = {}, []
        for entry in changed['resources']:
            block = original[entry['start']:entry['end']]
            payloads[entry['id']] = block[2:]
            if not block:
                continue
            decoded = decode_payload(block[2:], entry['flags'])
            if entry['rtype'] == 0x47:
                document = bitmap_document(decoded)
                rebuilt = encode_bitmap(document)
            elif name == 'AE001' and entry['index'] < 20:
                document = level_document(decoded)
                rebuilt = encode_level(document)
            else:
                continue
            if rebuilt != decoded or sha(rebuilt) != entry['decoded']['sha256']:
                raise ValueError(f"{entry['id']}: structured payload mismatch")
            encoded = encode_payload(rebuilt, entry['flags'])
            exact = encoded == block[2:]
            write_json(output / f'{entry["id"]}.json', document)
            rows.append({'id': entry['id'], 'format': document['format'], 'payload_bytes': len(rebuilt),
                         'payload_sha256': sha(rebuilt), 'payload_round_trip': 'EQUAL',
                         'compressed_round_trip': 'EQUAL' if exact else 'DIFFERS'})
            if promote and exact and entry.get('source_format') != document['format']:
                entry['kind'] = 'MATCHING_RESOURCE'
                entry['encoder'] = 'greedy-rle-pair-span-v1'
                entry['source_format'] = document['format']
                entry['source'] = f'raw/{name}/structured/{entry["index"]:03d}.json'
                outputs.append((project_path(root, entry['source']), document))
                payloads[entry['id']] = encoded
        trailing = original[manifest['trailing']['start']:] if manifest.get('trailing') else b''
        if assemble_archive(changed, payloads, trailing) != original:
            raise ValueError(f'{name}: structural promotion changed archive')
        plans.append((path, changed, outputs))
    changing = [p for p in plans if p[2]]
    if changing:
        for filename in ('game-report.json', 'archives-report.json', *(n + '.DAT' for n in NAMES)):
            (root / 'build' / filename).unlink(missing_ok=True)
        for path, manifest, outputs in changing:
            for target, document in outputs:
                target.parent.mkdir(parents=True, exist_ok=True)
                write_json(target, document)
            temporary = path.with_suffix('.json.tmp')
            write_json(temporary, manifest)
            temporary.replace(path)
            update_existing_recipe(root, path.stem, manifest)
    report = {'payloads_round_tripped': len(rows), 'payload_bytes': sum(r['payload_bytes'] for r in rows),
              'whole_resource_matches': sum(r['compressed_round_trip'] == 'EQUAL' for r in rows),
              'implementation_sha256': sha((root / 'tools/resource_formats.py').read_bytes()), 'resources': rows}
    write_json(root / 'build/resource-formats-report.json', report)
    print(f"{len(rows)} structured payloads round-trip exactly; {report['whole_resource_matches']} whole-resource matches")
    return report


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--promote', action='store_true')
    probe(promote=parser.parse_args().promote)
