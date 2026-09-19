"""Replace recovered runtime aliases with the historical CC.LIB public names."""
from pathlib import Path
import copy
import re

from mz import MZ
from reconstruct import (ROOT, read_json, write_json, owned_library_modules,
                         project_path)
from recover_library_bindings import derive as derive_library_bindings


# C identifier spellings. Turbo C adds the leading OMF underscore.
RENAMES = {
    'F_1D47': {'ff39c': 'memmove'},
    'F_233E': {'delay': 'f6c57'},
    'F_2AE2': {},
    'F_3986': {'ff98b': 'longjmp'},
    'F_462E': {'ff304': 'setmem', 'ff348': 'movmem'},
    'F_490D': {'ff8fe': 'srand', 'ffa0f': 'biostime'},
    'F_49F0': {'ff953': 'setjmp'},
    'F_520A': {'ff3da': 'farcoreleft', 'ff725': 'getdisk'},
    'F_56C6': {
        'rel': 'farfree',
        'delay': 'f6c57',
        'mode': 'f01ce',
        'getkey': 'f5593',
    },
    'F_622C': {'fF7CD': 'hardretn', 'fF7B2': 'hardresume'},
    'F_6266': {'fecda': 'open', 'feead': 'close', 'feefe': 'read'},
    'F_656C': {'feead': 'close', 'feefe': 'read', 'ff183': 'lseek', 'ff39c': 'memmove'},
    'F_8AA2': {'ff90f': 'rand'},
    'F_90A6': {'ff2db': 'strcpy'},
    'F_9259': {'ff3f7': 'free'},
    'F_9B68': {'ff90f': 'rand'},
    'F_A28D': {'ff25e': 'ultoa'},
    'F_A525': {'ff9be': 'toupper', 'ff304': 'setmem'},
    'F_A658': {'mode': 'f01ce', 'getkey': 'faf45'},
    'F_A85E': {'clr': 'setmem', 'put': 'strcpy', 'mode': 'f01ce'},
    'F_B4FB': {'ff304': 'setmem'},
    'F_B55E': {'free': 'farfree'},
    'F_B593': {'free': 'farfree'},
    'F_D3DA': {'ff3f7': 'free'},
    'F_D4B3': {'ff6c3': 'farfree'},
    'F_D8F0': {'ff32a': 'memset', 'ff348': 'movmem'},
}


def rename_source(path, mapping):
    original = path.read_bytes()
    text = original.decode('ascii')
    for old, new in mapping.items():
        text = text.replace(f'/*@SYM _{old}=', f'/*@SYM _{new}=')
        changed, count = re.subn(rf'\b{re.escape(old)}\b', new, text)
        if count:
            text = changed
        elif not re.search(rf'\b{re.escape(new)}\b', text):
            raise ValueError(f'{path}: neither {old} nor {new} is present')
    encoded = text.encode('ascii')
    if encoded != original:
        path.write_bytes(encoded)


def rename_bindings(owner, mapping):
    bindings = owner.get('build', {}).get('bindings', {})
    for old, new in mapping.items():
        old, new = '_' + old, '_' + new
        if old in bindings:
            if new in bindings:
                raise ValueError(f"{owner['id']}: binding collision at {new}")
            bindings[new] = bindings.pop(old)


def canonicalize_owned_binding(owner):
    """Express F_D825 references through its public instead of a numeric offset."""
    if owner.get('id') != 'F_2AE2':
        return
    binding = owner.get('build', {}).get('bindings', {}).get('_fd825')
    if binding:
        binding.clear()
        binding.update({
            'owner': 'F_D825',
            'public': '_fd825',
            'coordinate': 'code_offset',
            'addend': 0,
            'evidence': 'owned F_D825 public',
        })


def run():
    manifest_path = ROOT / 'layout/manifest.json'
    manifest = read_json(manifest_path)
    owners = {owner['id']: owner for owner in manifest['regions']}
    for owner_id, mapping in RENAMES.items():
        owner = owners[owner_id]
        if mapping:
            rename_source(ROOT / owner['source'], mapping)
        rename_bindings(owner, mapping)
        canonicalize_owned_binding(owner)
    original = project_path(ROOT, manifest['original']['path']).read_bytes()
    modules = owned_library_modules(
        manifest['regions'], ROOT / 'toolchain', read_json(ROOT / 'layout/toolchain.json'))
    manifest, library_changes = derive_library_bindings(manifest, MZ.parse(original), modules)
    canonical = {owner['id']: owner for owner in manifest['regions']}
    write_json(manifest_path, manifest)

    for path in sorted((ROOT / 'recipes/c').glob('*.json')):
        recipe = read_json(path)
        changed = False
        for owner in recipe.get('owners', []):
            mapping = RENAMES.get(owner['id'])
            if mapping:
                before = repr(owner.get('build', {}).get('bindings', {}))
                rename_bindings(owner, mapping)
                canonicalize_owned_binding(owner)
                for change in library_changes:
                    if change['caller'] == owner['id'] and change['symbol'] in owner['build']['bindings']:
                        owner['build']['bindings'][change['symbol']] = copy.deepcopy(
                            canonical[owner['id']]['build']['bindings'][change['symbol']])
                changed |= before != repr(owner.get('build', {}).get('bindings', {}))
        if changed:
            write_json(path, recipe)

    print(f'Promoted {sum(len(mapping) for mapping in RENAMES.values())} runtime references '
          f'across {len(RENAMES)} source owners and {len(library_changes)} library bindings')


if __name__ == '__main__':
    run()
