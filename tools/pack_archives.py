"""Pack DATs from ordered components; generation needs neither original files nor offsets."""
import argparse
import json
from pathlib import Path
import struct
import sys

from reconstruct import ROOT, project_path, read_json, sha, write_json
from resource_codecs import encode_payload
from resource_formats import encode_bitmap, encode_level

NAMES = ('AE000', 'AE001')
REPRESENTATIONS = ('opaque-encoded-fallback', 'decoded-bytes', 'bitmap4-json-v1', 'level-parts-json-v1')


def exact_keys(document, expected, context):
    if set(document) != set(expected):
        raise ValueError(f'{context}: unexpected or missing recipe fields: {sorted(set(document) ^ set(expected))}')


def validate_recipe(recipe):
    exact_keys(recipe, ('format', 'packing', 'resources', 'trailing'), 'archive')
    if recipe['format'] != 'empires-dat-build-v1' or recipe['packing'] != 'ordered-contiguous-le32-v1':
        raise ValueError('Unknown archive build/packing rules')
    if not isinstance(recipe['resources'], list):
        raise ValueError('Resources must be an ordered list')
    ids = set()
    for entry in recipe['resources']:
        if not isinstance(entry['id'], str) or not entry['id'] or entry['id'] in ids:
            raise ValueError('Resource IDs must be nonempty unique strings')
        ids.add(entry['id'])
        representation = entry['representation']
        if representation == 'empty':
            exact_keys(entry, ('id', 'representation'), entry['id'])
            continue
        if representation not in REPRESENTATIONS:
            raise ValueError(f"{entry['id']}: unsupported representation {representation}")
        fields = ['id', 'rtype', 'flags', 'source', 'representation']
        if representation != 'opaque-encoded-fallback':
            fields.append('encoder')
            if entry.get('encoder') != 'greedy-rle-pair-span-v1':
                raise ValueError(f"{entry['id']}: unsupported encoder policy")
        exact_keys(entry, fields, entry['id'])
        for key in ('rtype', 'flags'):
            if type(entry[key]) is not int or not 0 <= entry[key] <= 255:
                raise ValueError(f"{entry['id']}: {key} must be an unsigned byte")
        if not isinstance(entry['source'], str) or not entry['source']:
            raise ValueError('Missing component source path')
    if recipe['trailing'] is not None:
        exact_keys(recipe['trailing'], ('source', 'representation'), 'trailing')
        if recipe['trailing']['representation'] != 'opaque-encoded-fallback':
            raise ValueError('Unsupported trailing component representation')


def pack_blocks(blocks, trailing=b''):
    """Derive the complete table from count and emitted block lengths."""
    cursor = 4 * (len(blocks) + 1)
    offsets = [cursor]
    for block in blocks:
        if len(block) == 1:
            raise ValueError('A nonempty resource needs a two-byte header')
        cursor += len(block)
        if cursor > 0xFFFFFFFF:
            raise ValueError('Archive exceeds its 32-bit offsets')
        offsets.append(cursor)
    if offsets[0] > 0xFFFFFFFF:
        raise ValueError('Offset table exceeds 32-bit size')
    table = struct.pack(f'<{len(offsets)}I', *offsets)
    return table + b''.join(blocks) + trailing, offsets


def build_archive(root, recipe):
    validate_recipe(recipe)
    blocks, rows = [], []
    for entry in recipe['resources']:
        representation = entry['representation']
        source = b'' if representation == 'empty' else project_path(root, entry['source']).read_bytes()
        payload = source
        if representation == 'bitmap4-json-v1':
            payload = encode_bitmap(json.loads(source))
        elif representation == 'level-parts-json-v1':
            payload = encode_level(json.loads(source))
        if representation not in ('empty', 'opaque-encoded-fallback'):
            payload = encode_payload(payload, entry['flags'])
        block = b'' if representation == 'empty' else bytes((entry['rtype'], entry['flags'])) + payload
        blocks.append(block)
        rows.append({'id': entry['id'], 'representation': representation,
                     'source_sha256': sha(source), 'resource_sha256': sha(block), 'bytes': len(block)})
    trailing = project_path(root, recipe['trailing']['source']).read_bytes() if recipe['trailing'] else b''
    packed, offsets = pack_blocks(blocks, trailing)
    for i, row in enumerate(rows):
        row.update(start=offsets[i], end=offsets[i + 1])
    return packed, {'status': 'BUILT_UNVERIFIED', 'layout_mode': 'derived_from_component_order_and_sizes',
                    'original_files_read': False, 'historical_offsets_read': False,
                    'total_bytes': len(packed), 'sha256': sha(packed), 'generated_offsets': offsets,
                    'opaque_fallback_resources': sum(r['representation'] == 'opaque-encoded-fallback' for r in rows),
                    'opaque_fallback_resource_bytes': sum(r['bytes'] for r in rows if r['representation'] == 'opaque-encoded-fallback'),
                    'unstructured_decoded_resources': sum(r['representation'] == 'decoded-bytes' for r in rows),
                    'structured_resources': sum(r['representation'] in ('bitmap4-json-v1', 'level-parts-json-v1') for r in rows),
                    'trailing_bytes': len(trailing), 'resources': rows}


def pack(root=ROOT, output=None):
    output = output or root / 'build/packed'
    output.mkdir(parents=True, exist_ok=True)
    if output.resolve() == (root / 'build/packed').resolve():
        (root / 'build/game-report.json').unlink(missing_ok=True)
    for filename in ('packing-report.json', 'verification.json', *(n + '.DAT' for n in NAMES)):
        (output / filename).unlink(missing_ok=True)
    built, reports = {}, {}
    for name in NAMES:
        path = root / f'recipes/archives/{name}.json'
        built[name], reports[name] = build_archive(root, read_json(path))
        reports[name]['recipe_sha256'] = sha(path.read_bytes())
    for name, data in built.items():
        (output / f'{name}.DAT').write_bytes(data)
        print(f'{name}.DAT: packed {len(data):,} bytes; offsets derived from {len(reports[name]["resources"])} components')
    report = {'status': 'BUILT_UNVERIFIED', 'archives': reports,
              'implementation_sha256': {name: sha((root / 'tools' / name).read_bytes())
                                         for name in ('pack_archives.py', 'resource_codecs.py', 'resource_formats.py')}}
    write_json(output / 'packing-report.json', report)
    return report


def verify(root=ROOT, output=None, fixed_output=None):
    """Oracle reads occur only here, after component generation has finished."""
    from archive_recipe import recipe_from_layout
    from dat_archive import validate_manifest
    output = output or root / 'build/packed'
    (output / 'verification.json').unlink(missing_ok=True)
    if fixed_output is not None and fixed_output.resolve() == output.resolve():
        raise ValueError('Fixed and derived outputs must be separate for independent comparison')
    report = read_json(output / 'packing-report.json')
    for filename, digest in report['implementation_sha256'].items():
        if sha((root / 'tools' / filename).read_bytes()) != digest:
            raise ValueError(f'Packing report predates implementation changes: {filename}')
    checks = {}
    for name in NAMES:
        manifest = read_json(root / f'layout/archives/{name}.json')
        validate_manifest(manifest)
        recipe_path = root / f'recipes/archives/{name}.json'
        if read_json(recipe_path) != recipe_from_layout(manifest):
            raise ValueError(f'{name}: component sources/order differ between recipe and fixed scaffold')
        if sha(recipe_path.read_bytes()) != report['archives'][name]['recipe_sha256']:
            raise ValueError(f'{name}: packing report predates recipe changes')
        recipe = read_json(recipe_path)
        for entry, receipt in zip(recipe['resources'], report['archives'][name]['resources']):
            source = b'' if entry['representation'] == 'empty' else project_path(root, entry['source']).read_bytes()
            if sha(source) != receipt['source_sha256']:
                raise ValueError(f"{entry['id']}: packing report predates component source changes")
        actual = (output / f'{name}.DAT').read_bytes()
        if sha(actual) != report['archives'][name]['sha256']:
            raise ValueError(f'{name}: packed file differs from build receipt')
        original = project_path(root, manifest['original']['path']).read_bytes()
        if len(original) != manifest['original']['size'] or sha(original) != manifest['original']['sha256']:
            raise ValueError(f'{name}: original fixture identity differs')
        if actual != original:
            at = next((i for i, (a, b) in enumerate(zip(original, actual)) if a != b), min(len(original), len(actual)))
            raise ValueError(f'{name}: packed/original first mismatch at file offset 0x{at:X}')
        fixed = (fixed_output / f'{name}.DAT').read_bytes() if fixed_output else None
        if fixed is not None and fixed != actual:
            raise ValueError(f'{name}: fixed and derived packing differ')
        checks[name] = {'original_equal': True, 'fixed_equal': True if fixed is not None else None, 'sha256': sha(actual)}
    verified = {'status': 'EQUAL', 'archives': checks,
                'packing_report_sha256': sha((output / 'packing-report.json').read_bytes()),
                'whole_build_reconstruction_complete': False}
    write_json(output / 'verification.json', verified)
    print('Derived archive packing: EQUAL to originals' + (' and fixed scaffold' if fixed_output else ''))
    return verified


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('command', nargs='?', choices=('build', 'verify'), default='build')
    parser.add_argument('--output', type=Path, default=ROOT / 'build/packed')
    parser.add_argument('--fixed-output', type=Path)
    args = parser.parse_args()
    if not args.output.resolve().is_relative_to((ROOT / 'build').resolve()):
        parser.error('--output must be inside the generated build directory')
    try:
        if args.command == 'build':
            pack(ROOT, args.output.resolve())
        else:
            verify(ROOT, args.output.resolve(), args.fixed_output.resolve() if args.fixed_output else None)
    except (ValueError, OSError, KeyError) as error:
        print(f'FAIL: {error}', file=sys.stderr)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
