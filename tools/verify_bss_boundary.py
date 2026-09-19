"""Verify the source BSS reserve from independent historical object fixups."""
from mz import MZ
from omf import OmfReader
from reconstruct import ROOT, owned_library_modules, read_json, sha, write_json


def verify():
    manifest = read_json(ROOT / 'layout/manifest.json')
    recipe = read_json(ROOT / 'src/data/GAME_BSS.json')
    original = (ROOT / manifest['original']['path']).read_bytes()
    if sha(original) != manifest['original']['sha256']:
        raise ValueError('Oracle identity differs')
    mz = MZ.parse(original)
    frame = manifest['frames']['DGROUP']
    owners = {o['id']: o for o in manifest['regions']}
    modules = owned_library_modules(manifest['regions'], ROOT / 'toolchain',
                                    read_json(ROOT / 'layout/toolchain.json'))
    witnesses = []
    for owner_id, target in [('LIB_EXIT', '__atexittbl'), ('LIB_ATEXIT', '_BSS'), ('LIB_HARDERR', '_BSS')]:
        owner, module = owners[owner_id], modules[owner_id]
        values = []
        data = module.segment_bytes('_TEXT')
        for fixup in module.fixups_in('_TEXT'):
            if fixup['target'] != target:
                continue
            if fixup['loc'] != 'offset16' or fixup['self_relative']:
                raise ValueError('Unexpected runtime BSS reference form')
            offset = fixup['offset']
            raw = int.from_bytes(original[owner['start'] + offset:owner['start'] + offset + 2], 'little')
            addend = int.from_bytes(data[offset:offset + 2], 'little')
            value = (raw - addend - fixup['displacement']) & 65535
            values.append(value)
            witnesses.append({'owner': owner_id, 'target': target, 'load_site': owner['start'] - 512 + offset,
                              'recovered_dgroup_offset': value})
        if not values or len(set(values)) != 1:
            raise ValueError('Runtime BSS references do not corroborate one base')
    bases = {w['owner']: w['recovered_dgroup_offset'] for w in witnesses}
    if bases['LIB_EXIT'] != bases['LIB_ATEXIT']:
        raise ValueError('EXIT external and ATEXIT segment references disagree')
    atexit_size = modules['LIB_ATEXIT'].segment_length('_BSS')
    harderr_size = modules['LIB_HARDERR'].segment_length('_BSS')
    if bases['LIB_HARDERR'] != bases['LIB_ATEXIT'] + atexit_size:
        raise ValueError('Runtime BSS contributions are not contiguous')
    bss_start = len(mz.load_image(original)) - frame
    if recipe['length'] != bases['LIB_ATEXIT'] - bss_start:
        raise ValueError('Game BSS reserve differs from independently recovered boundary')
    end = bases['LIB_HARDERR'] + harderr_size
    startup = OmfReader().read((ROOT / 'toolchain/C0C.OBJ').read_bytes())
    fixup = next(f for f in startup.fixups_in('_TEXT') if f['target'] == '_BSSEND')
    at = fixup['offset']
    observed_end = int.from_bytes(mz.load_image(original)[at:at + 2], 'little')
    observed_end -= int.from_bytes(startup.segment_bytes('_TEXT')[at:at + 2], 'little') + fixup['displacement']
    if observed_end != end:
        raise ValueError('Startup BSSEND reference differs from runtime contribution end')
    result = {'status': 'EQUAL', 'game_reserve_bytes': recipe['length'],
              'runtime_bss_bytes': atexit_size + harderr_size,
              'dgroup_bss_start': bss_start, 'dgroup_bss_end': end,
              'witnesses': witnesses, 'startup_bssend_load_site': at,
              'internal_game_bss_partition_recovered': False}
    write_json(ROOT / 'docs/bss-boundary.json', result)
    print(f"BSS boundary: {recipe['length']} game bytes + {atexit_size + harderr_size} runtime bytes EQUAL")
    return result


if __name__ == '__main__':
    verify()
