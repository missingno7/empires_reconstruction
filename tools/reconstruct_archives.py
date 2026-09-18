"""Prepare local resource sources or rebuild both complete DAT archives exactly."""
import argparse
from collections import Counter
import json
import sys

from dat_archive import make_manifest, validate_manifest, assemble_archive
from reconstruct import ROOT, read_json, write_json, project_path, sha, mismatch
from resource_codecs import decode_payload, encode_payload
from resource_formats import SOURCE_FORMATS, decode_document, encode_document
from bitmap_sources import BUILD_SOURCE_FORMATS, encode_source, source_files

NAMES = ('AE000', 'AE001')


def initialize(root=ROOT):
    """One-time archive partitioning, never part of a normal build."""
    for name in NAMES:
        path = root / f'layout/archives/{name}.json'
        if path.exists():
            print(f'{name}: manifest already exists; left unchanged')
            continue
        data = (root / f'assets/{name}.DAT').read_bytes()
        manifest = make_manifest(name, data)
        for entry in manifest['resources']:
            if entry['kind'] == 'EMPTY_RESOURCE':
                continue
            block = data[entry['start']:entry['end']]
            decoded = decode_payload(block[2:], entry['flags'])
            entry['decoded'] = {'size': len(decoded), 'sha256': sha(decoded)}
        path.parent.mkdir(parents=True, exist_ok=True)
        write_json(path, manifest)
        print(f'{name}: {len(manifest["resources"])} resources, {len(data):,} bytes partitioned')


def prepare(root=ROOT):
    """Restore ignored local sources from the user's pinned original archives."""
    for name in NAMES:
        manifest = read_json(root / f'layout/archives/{name}.json')
        validate_manifest(manifest)
        original = project_path(root, manifest['original']['path']).read_bytes()
        if len(original) != manifest['original']['size'] or sha(original) != manifest['original']['sha256']:
            raise ValueError(f'{name}: original archive identity differs')
        outputs = []
        for entry in manifest['resources']:
            target = project_path(root, entry['source'])
            if not target.is_relative_to((root / 'raw').resolve()):
                raise ValueError('Archive extraction may only write local raw sources')
            block = original[entry['start']:entry['end']]
            if sha(block) != entry['expected_sha256']:
                raise ValueError(f"{entry['id']}: original resource identity differs")
            payload = block[2:]
            if entry['kind'] == 'MATCHING_RESOURCE':
                payload = decode_payload(payload, entry['flags'])
                if entry.get('source_format') == 'bitmap4-png-v1':
                    document = decode_document(name, entry, payload)
                    if document is None or document['format'] != 'bitmap4-json-v1':
                        raise ValueError('PNG source needs a standalone bitmap')
                    document, png = source_files(document, target.with_suffix('.png').name)
                    outputs.append((target.with_suffix('.png'), png))
                    payload = (json.dumps(document, indent=2) + '\n').encode('utf-8')
                elif entry.get('source_format') in SOURCE_FORMATS:
                    document = decode_document(name, entry, payload)
                    if document is None or document['format'] != entry['source_format']:
                        raise ValueError(f"{entry['id']}: source format does not match decoded structure")
                    payload = (json.dumps(document, indent=2) + '\n').encode('utf-8')
            outputs.append((target, payload))
        trailing = manifest.get('trailing')
        if trailing:
            target = project_path(root, trailing['source'])
            if not target.is_relative_to((root / 'raw').resolve()):
                raise ValueError('Trailing extraction may only write local raw sources')
            outputs.append((target, original[trailing['start']:trailing['end']]))
        for target, payload in outputs:
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(payload)
        print(f'{name}: restored {len(outputs)} ignored local resource sources')


def rebuild(root=ROOT, output=None):
    output = output or root / 'build'
    output.mkdir(parents=True, exist_ok=True)
    (output / 'archives-report.json').unlink(missing_ok=True)
    (output / 'game-report.json').unlink(missing_ok=True)
    for name in NAMES:
        (output / f'{name}.DAT').unlink(missing_ok=True)
    reports, built = {}, {}
    for name in NAMES:
        manifest = read_json(root / f'layout/archives/{name}.json')
        validate_manifest(manifest)
        original = project_path(root, manifest['original']['path']).read_bytes()
        if sha(original) != manifest['original']['sha256'] or len(original) != manifest['original']['size']:
            raise ValueError(f'{name}: original archive identity differs')
        payloads, rows = {}, []
        for entry in manifest['resources']:
            source = project_path(root, entry['source']).read_bytes()
            payload = source
            if entry['kind'] == 'MATCHING_RESOURCE':
                if entry.get('encoder') != 'greedy-rle-pair-span-v1':
                    raise ValueError(f"{entry['id']}: unknown encoder recipe")
                source_format = entry.get('source_format', 'decoded-bytes')
                if source_format == 'bitmap4-png-v1':
                    payload = encode_source(json.loads(source), project_path(root, entry['source']))
                elif source_format in SOURCE_FORMATS:
                    document = json.loads(source)
                    if document['format'] != source_format:
                        raise ValueError(f"{entry['id']}: structured source format differs")
                    payload = encode_document(document)
                elif source_format != 'decoded-bytes':
                    raise ValueError(f"{entry['id']}: unknown source format {source_format}")
                if sha(payload) != entry['decoded']['sha256'] or len(payload) != entry['decoded']['size']:
                    raise ValueError(f"{entry['id']}: decoded source identity differs")
                payload = encode_payload(payload, entry['flags'])
            expected = original[entry['start']:entry['end']]
            block = bytes([entry['rtype'], entry['flags']]) + payload if entry['kind'] != 'EMPTY_RESOURCE' else b''
            mismatch(expected, block, entry)
            decoded = decode_payload(payload, entry['flags']) if block else b''
            if block and (len(decoded) != entry['decoded']['size'] or sha(decoded) != entry['decoded']['sha256']):
                raise ValueError(f"{entry['id']}: decoded payload differs from established identity")
            document = decode_document(name, entry, decoded) if block else None
            structured_format = document['format'] if document else None
            if document and encode_document(document) != decoded:
                raise ValueError(f"{entry['id']}: structured payload round trip differs")
            payloads[entry['id']] = payload
            rows.append({'id': entry['id'], 'kind': entry['kind'], 'rtype': entry['rtype'], 'flags': entry['flags'],
                         'start': entry['start'], 'end': entry['end'], 'encoded_bytes': len(block),
                         'decoded_bytes': len(decoded), 'source_sha256': sha(source),
                         'structured_payload_format': structured_format,
                         'resource_sha256': sha(block), 'decoded_sha256': sha(decoded), 'status': 'EQUAL'})
        trailing = project_path(root, manifest['trailing']['source']).read_bytes() if manifest.get('trailing') else b''
        rebuilt = assemble_archive(manifest, payloads, trailing)
        if rebuilt != original:
            raise ValueError(f'{name}: whole-archive byte mismatch')
        matched = [r for r in rows if r['kind'] == 'MATCHING_RESOURCE']
        compressed_matched = [r for r in matched if r['flags'] & 3]
        reports[name] = {'status': 'EQUAL', 'total_bytes': len(rebuilt), 'accounted_bytes': len(rebuilt),
                         'structured_table_and_header_bytes': manifest['table']['offsets'][0] + 2 * sum(r['end'] > r['start'] for r in rows),
                         'resources_partitioned': len(rows), 'resources_decoded': sum(r['end'] > r['start'] for r in rows),
                         'decoded_payload_bytes': sum(r['decoded_bytes'] for r in rows),
                         'structured_payloads_rebuildable': sum(r['structured_payload_format'] is not None for r in rows),
                         'structured_payload_bytes_rebuildable': sum(r['decoded_bytes'] for r in rows if r['structured_payload_format']),
                         'structured_payload_format_counts': dict(Counter(r['structured_payload_format'] for r in rows if r['structured_payload_format'])),
                         'structured_source_format_counts': dict(Counter(r['source_format'] for r in manifest['resources'] if r.get('source_format') in BUILD_SOURCE_FORMATS)),
                         'resources_rebuildable': len(matched), 'resources_exact_matching': len(matched),
                         'resource_bytes_exact_matching': sum(r['encoded_bytes'] for r in matched),
                         'compressed_bytes_exact_matching': sum(r['encoded_bytes'] - 2 for r in compressed_matched),
                         'structured_asset_sources': sum(r.get('source_format') in BUILD_SOURCE_FORMATS for r in manifest['resources']),
                         'raw_resources': sum(r['kind'] == 'RAW_RESOURCE' for r in rows),
                         'raw_resource_bytes': sum(r['encoded_bytes'] for r in rows if r['kind'] == 'RAW_RESOURCE'),
                         'type_counts': dict(Counter(f'0x{r["rtype"]:02X}' for r in rows if r['rtype'] is not None)),
                         'original_sha256': sha(original), 'reconstructed_sha256': sha(rebuilt),
                         'manifest_sha256': sha((root / f'layout/archives/{name}.json').read_bytes()), 'resources': rows}
        reports[name]['implementation_sha256'] = {
            path: sha((root / 'tools' / path).read_bytes())
            for path in ('resource_codecs.py', 'resource_formats.py', 'bitmap_sources.py', 'indexed_png.py', 'dat_archive.py', 'reconstruct_archives.py')}
        built[name] = rebuilt
        print(f'{name}.DAT: EQUAL; {len(rebuilt):,} bytes, {len(rows)} resources, {len(matched)} re-encoded exactly, '
              f'{reports[name]["raw_resources"]} raw fallback')
    for name, data in built.items():
        (output / f'{name}.DAT').write_bytes(data)
    write_json(output / 'archives-report.json', reports)
    return reports


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('command', nargs='?', choices=('build', 'init', 'prepare'), default='build')
    args = parser.parse_args()
    try:
        {'build': rebuild, 'init': initialize, 'prepare': prepare}[args.command]()
    except (ValueError, OSError, KeyError) as error:
        print(f'FAIL: {error}', file=sys.stderr)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
