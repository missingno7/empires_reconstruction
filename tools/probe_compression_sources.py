"""Export/build independent decoded sources with explicit compression syntax.

This is a research path; canonical archive ownership and encoder-policy counts
are unchanged. Build reads only the exported directory, never original files.
"""
import argparse
from pathlib import Path

from compression_source import decode_source, encode_source
from pack_archives import pack_blocks
from reconstruct import ROOT, read_json, write_json, project_path, sha
from resource_codecs import unpack_rle, pack_rle
from resource_formats import decode_document, encode_document


def build(directory):
    recipe = read_json(directory / 'sources.json')
    if recipe['format'] != 'empires-compression-sources-v1':
        raise ValueError('Unknown compression source recipe')
    result = {}
    for name, archive in recipe['archives'].items():
        if name not in ('AE000', 'AE001'):
            raise ValueError('Unknown archive name')
        blocks = []
        for entry in archive['resources']:
            if entry['empty']:
                blocks.append(b'')
                continue
            flags = entry['flags']
            if type(flags) is not int or flags & ~3:
                raise ValueError('Unknown compression flags')
            source = project_path(directory, entry['source'])
            data = encode_document(read_json(source)) if entry['structured'] else source.read_bytes()
            if flags & 1:
                data = pack_rle(data)
            if bool(flags & 2) != bool(entry['compression']):
                raise ValueError('Compression source does not match flags')
            if flags & 2:
                data = encode_source(data, read_json(project_path(directory, entry['compression'])))
            blocks.append(bytes((entry['rtype'], flags)) + data)
        trailing = project_path(directory, archive['trailing']).read_bytes() if archive['trailing'] else b''
        result[name] = pack_blocks(blocks, trailing)[0]
    return result


def export(root, directory):
    if directory.exists():
        raise ValueError('Export directory already exists; choose a fresh directory to preserve edits')
    directory.mkdir(parents=True)
    recipe = {'format': 'empires-compression-sources-v1', 'archives': {}}
    originals, rows = {}, []
    for name in ('AE000', 'AE001'):
        manifest = read_json(root / f'layout/archives/{name}.json')
        original = project_path(root, manifest['original']['path']).read_bytes()
        if sha(original) != manifest['original']['sha256']:
            raise ValueError('Original identity differs')
        originals[name] = original
        archive = {'resources': [], 'trailing': None}
        recipe['archives'][name] = archive
        for entry in manifest['resources']:
            block = original[entry['start']:entry['end']]
            if sha(block) != entry['expected_sha256']:
                raise ValueError('Resource identity differs')
            spec = {'empty': not bool(block)}
            archive['resources'].append(spec)
            if not block:
                continue
            flags, data = entry['flags'], block[2:]
            stem = entry['id']
            plan_name = None
            if flags & 2:
                data, plan = decode_source(data)
                if encode_source(data, plan) != block[2:]:
                    raise ValueError('Compression syntax round trip differs')
                plan_name = stem + '.compression.json'
                write_json(directory / plan_name, plan)
            if flags & 1:
                decoded = unpack_rle(data)
                if pack_rle(decoded) != data:
                    raise ValueError('RLE policy differs; no hidden fallback permitted')
                data = decoded
            document = decode_document(name, entry, data)
            source = stem + ('.json' if document else '.bin')
            if document:
                if encode_document(document) != data:
                    raise ValueError('Structured payload round trip differs')
                write_json(directory / source, document)
            else:
                (directory / source).write_bytes(data)
            spec.update(rtype=entry['rtype'], flags=flags, source=source,
                        structured=document is not None, compression=plan_name)
            rows.append({'id': stem, 'structured': document is not None,
                         'explicit_compression_plan': plan_name is not None,
                         'decoded_bytes': len(data), 'encoded_bytes': len(block) - 2})
        if manifest.get('trailing'):
            archive['trailing'] = name + '.trailing.bin'
            (directory / archive['trailing']).write_bytes(original[manifest['trailing']['start']:])
    write_json(directory / 'sources.json', recipe)
    rebuilt = build(directory)
    if rebuilt != originals:
        raise ValueError('Independent source archives differ')
    report = {'status': 'EQUAL', 'historical_compressor_policy_recovered': False,
              'canonical_ownership_changed': False, 'resources': rows,
              'structured_payloads': sum(r['structured'] for r in rows),
              'opaque_decoded_payloads': sum(not r['structured'] for r in rows),
              'explicit_compression_plans': sum(r['explicit_compression_plan'] for r in rows),
              'archives': {name: {'bytes': len(data), 'sha256': sha(data)} for name, data in rebuilt.items()}}
    write_json(directory / 'verification.json', report)
    print(f"EQUAL: {report['explicit_compression_plans']} compression plans; {report['structured_payloads']} structured and {report['opaque_decoded_payloads']} opaque decoded payloads")
    return report


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('mode', choices=('export', 'build'))
    parser.add_argument('--directory', type=Path, default=ROOT / 'build/compression-sources')
    args = parser.parse_args()
    directory = args.directory.resolve()
    if not directory.is_relative_to((ROOT / 'build').resolve()):
        parser.error('Use a directory within project build/')
    if args.mode == 'export':
        export(ROOT, directory)
    else:
        for name in ('AE000', 'AE001'):
            (directory / (name + '.DAT')).unlink(missing_ok=True)
        for name, data in build(directory).items():
            (directory / (name + '.DAT')).write_bytes(data)
            print(f'{name}: BUILT_UNVERIFIED; {len(data)} bytes; {sha(data)}')
