"""Make reconstructed callers use the selected public of each code owner."""
import copy
from pathlib import Path
import re

from reconstruct import ROOT, read_json, write_json


ASM_ENTRY_RENAMES = {
    'F_C27D': {'fc2ea': 'f_c2ea', 'fc440': 'f_c440',
               'fc6b9': 'f_c6b9', 'fc755': 'f_c755'},
    'F_C834': {'fc6b9': 'f_c6b9'},
    'F_C877': {'fc6b9': 'f_c6b9'},
    'F_C914': {'fc9a4': 'f_c9a4', 'fca03': 'f_ca03'},
}


def entry_renames(manifest):
    owners = {owner['id']: owner for owner in manifest['regions']}
    result = {}
    for caller in manifest['regions']:
        if caller.get('kind') != 'MATCHING_C':
            continue
        for symbol, binding in caller.get('build', {}).get('bindings', {}).items():
            target = owners.get(binding.get('owner'))
            if (not target or target.get('kind') != 'MATCHING_C'
                    or target.get('provenance', {}).get('resolves_held_candidate')
                    or binding.get('coordinate') != 'code_offset'
                    or binding.get('addend', 0) != 0):
                continue
            public = target.get('build', {}).get('public')
            if public and public != symbol:
                result.setdefault(caller['id'], {})[symbol] = public
    return result


def rename_source(path, mapping):
    original = path.read_bytes()
    text = original.decode('ascii')
    for old, new in mapping.items():
        old_c, new_c = old[1:], new[1:]
        text = text.replace(f'/*@SYM {old}=', f'/*@SYM {new}=')
        text, count = re.subn(
            rf'\b{re.escape(old_c)}\b(?=\s*\()', new_c, text)
        if not count and not re.search(rf'\b{re.escape(new_c)}\b(?=\s*\()', text):
            raise ValueError(f'{path}: neither callable {old_c} nor {new_c} is present')
    encoded = text.encode('ascii')
    if encoded != original:
        path.write_bytes(encoded)


def rename_bindings(owner, mapping):
    bindings = owner.get('build', {}).get('bindings', {})
    for old, new in mapping.items():
        if old not in bindings:
            continue
        replacement = bindings.pop(old)
        replacement['public'] = new
        if new in bindings:
            identity = lambda value: (value.get('coordinate'), value.get('owner'),
                                      value.get('public'), value.get('addend', 0))
            if identity(bindings[new]) != identity(replacement):
                raise ValueError(f"{owner['id']}: incompatible binding collision at {new}")
        else:
            bindings[new] = replacement


def run():
    manifest_path = ROOT / 'layout/manifest.json'
    manifest = read_json(manifest_path)
    owners = {owner['id']: owner for owner in manifest['regions']}
    runtime = owners['RUNTIME_BLOCK']
    runtime_path = ROOT / runtime['source']
    runtime_path.write_bytes(runtime_path.read_bytes().replace(
        b'void runtime_block()', b'void f039c()', 1))
    runtime['build']['public'] = '_f039c'
    mappings = entry_renames(manifest)
    for owner_id, mapping in mappings.items():
        rename_source(ROOT / owners[owner_id]['source'], mapping)
        rename_bindings(owners[owner_id], mapping)
    for owner_id, mapping in ASM_ENTRY_RENAMES.items():
        path = ROOT / owners[owner_id]['source']
        original = path.read_bytes()
        text = original.decode('ascii')
        for old, new in mapping.items():
            text = re.sub(rf'\b{re.escape(old)}\b', new, text)
        encoded = text.encode('ascii')
        if encoded != original:
            path.write_bytes(encoded)

    # C0C.OBJ's conventional entry reference is the strongest name evidence.
    main = owners['F_4A93']
    main_path = ROOT / main['source']
    main_text = main_path.read_bytes().replace(b'f4a93()', b'main()', 1)
    main_path.write_bytes(main_text)
    main['build']['public'] = '_main'
    write_json(manifest_path, manifest)

    changed_recipes = 0
    for path in sorted((ROOT / 'recipes/c').glob('*.json')):
        recipe = read_json(path)
        changed = False
        for owner in recipe.get('owners', []):
            mapping = mappings.get(owner['id'])
            if mapping:
                before = repr(owner.get('build', {}).get('bindings', {}))
                rename_bindings(owner, mapping)
                changed |= before != repr(owner.get('build', {}).get('bindings', {}))
            if owner.get('id') == 'F_4A93':
                owner['build']['public'] = '_main'
                changed = True
            if owner.get('id') == 'RUNTIME_BLOCK':
                owner['build']['public'] = '_f039c'
                changed = True
        if changed:
            write_json(path, recipe)
            changed_recipes += 1
    print(f'Canonicalized {sum(len(value) for value in mappings.values())} entry references '
          f'across {len(mappings)} callers and {changed_recipes} recipes; '
          'entries are _main and _f039c')


if __name__ == '__main__':
    run()
