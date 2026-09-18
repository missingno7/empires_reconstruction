"""Promote the raw MZ header to explicit, lossless metadata without changing layout."""
import copy

from mz import MZ, encode_header, header_document
from reconstruct import ROOT, mismatch, project_path, read_json, sha, validate_layout, write_json


def main():
    manifest_path = ROOT / 'layout/manifest.json'
    manifest = read_json(manifest_path)
    validate_layout(manifest)
    previous = manifest['regions'][0]
    if previous['kind'] == 'MZ_HEADER':
        print('MZ header already has structured ownership; source left unchanged.')
        return
    original = project_path(ROOT, manifest['original']['path']).read_bytes()
    if sha(original) != manifest['original']['sha256'] or len(original) != manifest['original']['size']:
        raise ValueError('Original EXE identity mismatch')
    mz = MZ.parse(original)
    if previous['kind'] != 'RAW' or previous['start'] != 0 or previous['end'] != mz.header_size:
        raise ValueError('Expected one RAW owner for the complete MZ header')
    document = header_document(original)
    encoded = encode_header(document, len(original))
    mismatch(original[:mz.header_size], encoded, previous)
    if sha(encoded) != previous['expected_sha256']:
        raise ValueError('Original header digest differs from manifest')
    target = ROOT / 'layout/mz-header.json'
    if target.exists() and read_json(target) != document:
        raise ValueError('Refusing to overwrite existing header source')
    proposed = copy.deepcopy(manifest)
    owner = proposed['regions'][0]
    owner.update(id='MZ_HEADER', kind='MZ_HEADER', source='layout/mz-header.json',
                 artifact='regions/MZ_HEADER.bin', original_symbol=None)
    owner['provenance'] = {'decoded_from': manifest['original']['path'],
                           'image_sha256': sha(original), 'previous_owner': previous['id']}
    validate_layout(proposed)
    # No canonical state changes before the candidate has passed exact comparison.
    for name in ('AEPROG.EXE', 'report.json'):
        (ROOT / 'build' / name).unlink(missing_ok=True)
    write_json(target, document)
    pending = manifest_path.with_suffix('.json.pending')
    write_json(pending, proposed)
    pending.replace(manifest_path)
    print(f'Promoted {mz.header_size} header bytes: 14 fields, {len(mz.relocations)} ordered relocations, '
          f'{len(encoded) - 28 - 4 * len(mz.relocations)} preserved reserved/padding bytes.')
    print('Run python tools/reconstruct.py for a fresh full-file proof.')


if __name__ == '__main__':
    main()
