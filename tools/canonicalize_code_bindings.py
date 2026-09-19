"""Replace numeric function-entry bindings with owned OMF public bindings."""
from pathlib import Path

from reconstruct import ROOT, read_json, write_json


def owner_entries(manifest):
    result = {}
    for owner in manifest['regions']:
        build = owner.get('build', {})
        public = build.get('public')
        if public and build.get('segment') == '_TEXT':
            result[owner['start'] - 512] = (owner['id'], public)
    return result


def canonicalize(document, entries):
    changed = 0
    for owner in document.get('regions', document.get('owners', [])):
        for binding in owner.get('build', {}).get('bindings', {}).values():
            if binding.get('coordinate') != 'code_offset' or 'offset' not in binding:
                continue
            target = entries.get(binding['offset'])
            if not target:
                continue
            target_owner, public = target
            evidence = binding.get('evidence', 'exact reconstructed function entry')
            binding.clear()
            binding.update({
                'coordinate': 'code_offset',
                'owner': target_owner,
                'public': public,
                'addend': 0,
                'evidence': evidence,
            })
            changed += 1
    return changed


def run():
    manifest_path = ROOT / 'layout/manifest.json'
    manifest = read_json(manifest_path)
    entries = owner_entries(manifest)
    total = canonicalize(manifest, entries)
    write_json(manifest_path, manifest)

    changed_recipes = 0
    for path in sorted((ROOT / 'recipes/c').glob('*.json')):
        recipe = read_json(path)
        changed = canonicalize(recipe, entries)
        if changed:
            write_json(path, recipe)
            changed_recipes += 1
            total += changed
    print(f'Canonicalized {total} function-entry bindings in the manifest and '
          f'{changed_recipes} recipes')


if __name__ == '__main__':
    run()
