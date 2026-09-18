"""Promote the two DAC tables referenced by freshly matched F_0281/F_01BC."""
import os
from pathlib import Path
import tempfile

from exe_data import DAC_FORMAT, palette_document, encode_data
from mz import MZ
from promote_upstream import replace_raw_owners
from reconstruct import (ROOT, read_json, write_json, sha, project_path,
                         compile_sources, read_object, bind_region, mismatch, owned_library_modules)


def promote(root=ROOT):
    path = root / 'layout/manifest.json'
    manifest = read_json(path)
    original = project_path(root, manifest['original']['path']).read_bytes()
    if sha(original) != manifest['original']['sha256'] or len(original) != manifest['original']['size']:
        raise ValueError('Original identity mismatch')
    mz = MZ.parse(original)
    frames = manifest['frames']
    candidates, documents = [], {}
    for offset in (0x11e, 0x41e):
        name = f'PALETTE_DAC6_{offset:04X}'
        start = mz.file_offset(frames['DGROUP'] + offset)
        data = original[start:start + 768]
        document = palette_document(data)
        if encode_data(document, DAC_FORMAT) != data:
            raise ValueError('Palette round trip differs')
        source = f'raw/exe-data/{name}.json'
        target = project_path(root, source)
        if target.exists() and read_json(target) != document:
            raise ValueError(f'Refusing to overwrite edited source: {source}')
        documents[source] = document
        candidates.append({'id': name, 'start': start, 'end': start + 768,
                           'kind': 'EXACT_DATA', 'classification': 'embedded_palette',
                           'source': source, 'artifact': f'regions/{name}.bin',
                           'expected_sha256': sha(data), 'matching_status': 'EQUAL',
                           'build': {'encoder': DAC_FORMAT}})
    existing = {r['id'] for r in manifest['regions']}
    for candidate in candidates:
        previous = next((r for r in manifest['regions'] if r['id'] == candidate['id']), None)
        if previous is not None and previous != candidate:
            raise ValueError(f"Existing palette ownership differs: {candidate['id']}")
    updated = replace_raw_owners(manifest, [r for r in candidates if r['id'] not in existing], original)
    owner = next(r for r in updated['regions'] if r['id'] == 'F_0281')
    for offset in (0x11e, 0x41e):
        symbol = f'_g{offset:x}'
        old = owner['build']['bindings'][symbol]
        if 'offset' in old and old['offset'] != offset:
            raise ValueError('Unexpected palette reference')
        owner['build']['bindings'][symbol] = {
            'owner': f'PALETTE_DAC6_{offset:04X}', 'addend': 0,
            'coordinate': 'DGROUP_offset',
            'evidence': 'F_0281 passes this table to F_01BC; INT 10h AX=1012h CX=0100h'}
    evidence_owners = [r for r in updated['regions'] if r['id'] in ('F_0281', 'F_01BC')]
    (root / 'build').mkdir(exist_ok=True)
    work = Path(tempfile.mkdtemp(prefix='palettes-', dir=root / 'build'))
    receipts, session = compile_sources(root, evidence_owners, work, root / 'toolchain',
        Path(os.environ.get('DOSBOX', 'C:/Program Files/DOSBox Staging/dosbox.exe')),
        read_json(root / 'layout/toolchain.json'))
    evidence = []
    component_modules = owned_library_modules(updated['regions'], root / 'toolchain', read_json(root / 'layout/toolchain.json'))
    for code in evidence_owners:
        module = read_object((work / receipts[code['id']]['object']).read_bytes())
        part, proof = bind_region(code, module, mz, frames, updated['regions'], component_modules)
        mismatch(original[code['start']:code['end']], part, code)
        evidence.append({'id': code['id'], 'source_sha256': sha(project_path(root, code['source']).read_bytes()),
                         'matched_sha256': sha(part)})
    # Publish only after both fresh object comparisons and all data checks pass.
    for name in ('AEPROG.EXE', 'report.json', 'game-report.json'):
        (root / 'build' / name).unlink(missing_ok=True)
    for source, document in documents.items():
        target = project_path(root, source)
        target.parent.mkdir(parents=True, exist_ok=True)
        write_json(target, document)
    for region in updated['regions']:
        if region['kind'] == 'RAW':
            target = project_path(root, region['source'])
            if not target.exists():
                target.parent.mkdir(parents=True, exist_ok=True)
                target.write_bytes(original[region['start']:region['end']])
    write_json(path, updated)
    write_json(root / 'docs/embedded-palettes.json', {
        'status': 'EQUAL', 'original_sha256': sha(original), 'evidence_functions': evidence,
        'components': candidates, 'palette_bytes': 1536,
        'address_model': 'component-owned bindings within fixed placement; not real linking'})
    print('EQUAL: two structured DAC tables, 1,536 bytes; two component-owned bindings')


if __name__ == '__main__':
    promote()
