"""Recreate ignored raw owners from a locally supplied, identity-checked EXE."""
from pathlib import Path
import json
from exe_data import decode_data, encode_data

from reconstruct import ROOT, project_path, read_json, sha, validate_layout


def main():
    manifest = read_json(ROOT / 'layout/manifest.json')
    validate_layout(manifest)
    original = project_path(ROOT, manifest['original']['path']).read_bytes()
    if len(original) != manifest['original']['size'] or sha(original) != manifest['original']['sha256']:
        raise SystemExit('Original EXE identity does not match the manifest; no files extracted')
    outputs = []
    raw_root = (ROOT / 'raw').resolve()
    for owner in manifest['regions']:
        if owner['kind'] not in ('RAW', 'EXACT_DATA'):
            continue
        if owner.get('build', {}).get('encoder') in ('omf-segment-v1', 'ascii-nul-v1', 'ascii-v1'):
            continue  # Canonical compiler/text sources must never be overwritten by extraction.
        target = project_path(ROOT, owner['source'])
        if not target.is_relative_to(raw_root):
            if owner['kind'] == 'EXACT_DATA':
                continue  # Structured sources under src/data are canonical too.
            raise SystemExit(f'Raw owner outside raw directory: {owner["id"]}')
        data = original[owner['start']:owner['end']]
        if sha(data) != owner['expected_sha256']:
            raise SystemExit(f'Extent digest mismatch: {owner["id"]}; no files extracted')
        if owner['kind'] == 'EXACT_DATA':
            document = decode_data(data, owner['build']['encoder'])
            if encode_data(document, owner['build']['encoder']) != data:
                raise SystemExit(f'Structured data round trip differs: {owner["id"]}')
            data = (json.dumps(document, indent=2) + '\n').encode('utf-8')
        outputs.append((target, data))
    for target, data in outputs:
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(data)
    print(f'Prepared {len(outputs)} local EXE sources (raw owners and structured data; {sum(len(data) for _, data in outputs):,} source bytes)')


if __name__ == '__main__':
    main()
