"""Recreate ignored raw owners from a locally supplied, identity-checked EXE."""
from pathlib import Path

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
        if owner['kind'] != 'RAW':
            continue
        target = project_path(ROOT, owner['source'])
        if not target.is_relative_to(raw_root):
            raise SystemExit(f'Raw owner outside raw directory: {owner["id"]}')
        data = original[owner['start']:owner['end']]
        if sha(data) != owner['expected_sha256']:
            raise SystemExit(f'Extent digest mismatch: {owner["id"]}; no files extracted')
        outputs.append((target, data))
    for target, data in outputs:
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(data)
    print(f'Extracted {len(outputs)} exact raw owners ({sum(len(data) for _, data in outputs):,} bytes)')


if __name__ == '__main__':
    main()
