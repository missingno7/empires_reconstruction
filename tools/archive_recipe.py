"""Component build recipes contain order/encoding rules, never original placement."""
from reconstruct import write_json


def recipe_from_layout(manifest):
    """Explicit migration/promotion helper. Never called by archive generation."""
    resources = []
    for entry in manifest['resources']:
        if entry['kind'] == 'EMPTY_RESOURCE':
            resources.append({'id': entry['id'], 'representation': 'empty'})
            continue
        row = {'id': entry['id'], 'rtype': entry['rtype'], 'flags': entry['flags'],
               'source': entry['source']}
        if entry['kind'] == 'RAW_RESOURCE':
            row['representation'] = 'opaque-encoded-fallback'
        elif entry['kind'] == 'MATCHING_RESOURCE':
            row['representation'] = entry.get('source_format', 'decoded-bytes')
            row['encoder'] = entry['encoder']
        else:
            raise ValueError(f"{entry['id']}: unsupported source ownership")
        resources.append(row)
    return {'format': 'empires-dat-build-v1', 'packing': 'ordered-contiguous-le32-v1',
            'resources': resources,
            'trailing': {'source': manifest['trailing']['source'], 'representation': 'opaque-encoded-fallback'}
            if manifest.get('trailing') else None}


def update_existing_recipe(root, name, manifest):
    """Keep adopted component sources in sync when a verified promotion occurs."""
    path = root / f'recipes/archives/{name}.json'
    if path.exists():
        for filename in ('packing-report.json', 'verification.json', 'AE000.DAT', 'AE001.DAT'):
            (root / 'build/packed' / filename).unlink(missing_ok=True)
        temporary = path.with_suffix('.json.tmp')
        write_json(temporary, recipe_from_layout(manifest))
        temporary.replace(path)
