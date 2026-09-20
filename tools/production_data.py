"""Canonical DATA/BSS emission extracted from the proven source-DATA experiment."""
from pathlib import Path
import os
import struct

from data_omf import emit_data
from bss_asm import bss_asm_source, bss_slice
from exe_data import encode_data
from omf import OmfReader
from pointer_records import FORMAT, compile_records
from sound_data import FORMAT as SOUND_FORMAT, compile_sound_data
from typed_data import FORMAT as TYPED_FORMAT, compile_typed_data
from reconstruct import read_json, sha



def prepare_data(root, work, manifest, staged, module_owners, member_objects):
    """Emit source DATA and BSS inputs; never link or read a previous receipt."""
    ROOT = root
    owners = {o['id']: o for o in manifest['regions']}
    owners.update({o['id']: o for o in module_owners})
    recipe = read_json(ROOT / 'recipes/data/game-initialized.json')
    parts = []
    for spec in recipe['components']:
        refs = []
        if spec['format'] == 'raw-local':
            data = bytearray((ROOT / spec['source']).read_bytes())
            for pointer in spec.get('symbolic_pointer_fields', []):
                if not 0 <= pointer['offset'] <= len(data) - 4:
                    raise ValueError('Raw source pointer lies outside its contribution')
                struct.pack_into('<HH', data, pointer['offset'], pointer['addend'], 0)
                refs.append({'offset': pointer['offset'], 'target': pointer['target']})
            data = bytes(data)
        elif spec['format'] == 'compiled-data':
            code = owners[spec['code_owner']]
            entry = member_objects[code['id']]
            module = OmfReader().read((work / 'WORK' / entry['object']).read_bytes())
            full_data = module.segment_bytes('_DATA')
            offset = entry['data_component_offsets'][spec['id']]
            data = full_data[offset:offset + owners[spec['id']]['end'] - owners[spec['id']]['start']]
            for fixup in ([] if spec.get('retain_in_code_object') else module.fixups_in('_DATA')):
                if fixup['loc'] != 'pointer32' or fixup['target_kind'] != 'external' or fixup['self_relative']:
                    raise ValueError('Unsupported compiled DATA reference')
                binding = code['build']['bindings'][fixup['target']]
                if binding.get('coordinate') != 'DGROUP_offset' or 'owner' not in binding:
                    raise ValueError('Compiled pointer needs component ownership')
                if binding.get('addend', 0) or fixup['displacement']:
                    raise ValueError('Compiled pointer addend requires explicit normalization')
                if offset <= fixup['offset'] < offset + len(data):
                    refs.append({'offset': fixup['offset'] - offset, 'target': binding['owner']})
        elif spec['format'] == TYPED_FORMAT:
            data, refs, typed_publics = compile_typed_data(read_json(ROOT / spec['source']))
        elif spec['format'] == FORMAT:
            data, refs = compile_records(read_json(ROOT / spec['source']))
        elif spec['format'] == SOUND_FORMAT:
            data, refs, sound_publics = compile_sound_data(read_json(ROOT / spec['source']))
        else:
            data = encode_data(read_json(ROOT / spec['source']), spec['format'])
        skip = spec.get('skip', 0)
        if skip and refs:
            raise ValueError('Cannot slice pointer contribution')
        data = data[skip:]
        parts.append({'spec': spec, 'data': data, 'refs': refs,
                      'publics': ({} if skip else
                                  sound_publics if spec['format'] == SOUND_FORMAT else
                                  typed_publics if spec['format'] == TYPED_FORMAT else {spec['id']: 0})})

    retained = {}
    for part in parts:
        spec = part['spec']
        if not spec.get('retain_in_code_object'):
            continue
        if spec['format'] != 'compiled-data' or spec.get('skip'):
            raise ValueError('Retained DATA requires a complete compiled contribution')
        entry = member_objects[spec['code_owner']]
        retained.setdefault(entry['id'], []).append(part)
    for ident, contributions in retained.items():
        entry = staged[ident]
        module = OmfReader().read((work / 'WORK' / entry['object']).read_bytes())
        cursor = 0
        for part in contributions:
            if entry['data_component_offsets'][part['spec']['id']] != cursor:
                raise ValueError('Retained DATA slices are not contiguous native ownership')
            part['native_offset'] = cursor
            cursor += len(part['data'])
        if module.segment_bytes('_DATA') != b''.join(p['data'] for p in contributions):
            raise ValueError('Retained DATA is not the complete native contribution')

    # Existing fixed binding evidence routes aliases to source components. It
    # supplies no placement directive or padding; TLINK concatenates sources.
    # This adapter remains explicitly temporary until historical declarations
    # and module storage ownership replace the recovered global names.
    frame = manifest['frames']['DGROUP']
    startup_path = work / 'TC/LIB/C0C.OBJ'
    startup = OmfReader().read(startup_path.read_bytes())
    startup_aliases = {}
    runtime_aliases = {}
    library = {name: OmfReader().read(blob) for name, blob in
               OmfReader().split_library((work / 'TC/LIB/CC.LIB').read_bytes())}
    bss_source = read_json(ROOT / 'src/data/GAME_BSS.json')
    if bss_source['format'] != 'anchored-bss-layout-v1' or bss_source['alignment'] != 'word':
        raise ValueError('Unsupported game BSS source')
    source_bss_publics = bss_source.get('publics')
    if (not isinstance(source_bss_publics, dict) or not source_bss_publics
            or any(not isinstance(name, str) or type(offset) is not int
                   or not 0 <= offset < bss_source['length']
                   for name, offset in source_bss_publics.items())):
        raise ValueError('Invalid canonical BSS public map')
    bss_plan = read_json(ROOT / 'recipes/data/bss-contributions.json')
    if (bss_plan.get('format') != 'empires-bss-contributions-v1'
            or bss_plan.get('canonical_layout') != 'src/data/GAME_BSS.json'):
        raise ValueError('Unsupported BSS contribution plan')
    bss_contributions = []
    expected_start = 0
    for contribution in bss_plan.get('contributions', []):
        start, end = contribution.get('logical_start'), contribution.get('logical_end')
        if start != expected_start:
            raise ValueError('BSS contributions must be contiguous and ordered')
        sliced = bss_slice(bss_source, start, end)
        if (not all(isinstance(contribution.get(key), str) for key in ('id', 'object', 'assembly'))
                or type(contribution.get('aggregate_storage')) is not bool):
            raise ValueError('Invalid BSS contribution identity')
        # The pinned TASM runs inside DOS, where generated source/object names
        # need an 8.3 spelling even though the host filesystem permits longer.
        object_name, assembly_name = contribution['object'], contribution['assembly']
        if (Path(object_name).suffix.upper() != '.OBJ' or Path(assembly_name).suffix.upper() != '.ASM'
                or len(Path(object_name).stem) > 8 or len(Path(assembly_name).stem) > 8):
            raise ValueError('BSS contribution source/object must use DOS 8.3 names')
        bss_contributions.append({**contribution, 'length': sliced['length'],
                                  'publics': sliced['publics']})
        expected_start = end
    if expected_start != bss_source['length']:
        raise ValueError('BSS contribution plan does not cover the canonical reserve')
    if (len({item['object'] for item in bss_contributions}) != len(bss_contributions)
            or len({item['assembly'] for item in bss_contributions}) != len(bss_contributions)):
        raise ValueError('BSS contribution objects and sources must be unique')
    def add_alias(symbol, dgroup_offset):
        if 0 <= dgroup_offset <= startup.segment_length('_DATA'):
            startup_aliases[symbol] = dgroup_offset
            return
        location = frame + 512 + dgroup_offset
        for part in parts:
            owner = owners[part['spec']['id']]
            start = owner['start'] + part['spec'].get('skip', 0)
            if start <= location < owner['end']:
                offset = location - start
                if symbol in part['publics'] and part['publics'][symbol] != offset:
                    raise ValueError('Conflicting component-relative alias')
                part['publics'][symbol] = offset
                return
        for owner in owners.values():
            if (owner['kind'] == 'KNOWN_TOOLCHAIN_LIBRARY' and owner['build']['segment'] == '_DATA'
                    and owner['start'] <= location < owner['end']):
                module = library[owner['build']['library_module']]
                offset = location - owner['start']
                public = max((p for p in module.publics_in('_DATA') if p['offset'] <= offset),
                             key=lambda p: p['offset'])
                runtime_aliases[symbol] = (public['name'], offset - public['offset'])
                return
        raise ValueError(f'No source component for temporary alias {symbol}')

    explicit_publics = {public['name'] for public in startup.publics}
    explicit_publics.update(public['name'] for module in library.values() for public in module.publics)
    for entry in staged.values():
        explicit_publics.update(public['name'] for public in
                                OmfReader().read((work / 'WORK' / entry['object']).read_bytes()).publics)
    explicit_casefold = {name.lower() for name in explicit_publics}
    initialized_span = max(owner['end'] for owner in manifest['regions']) - 512 - frame
    bss_public_offsets = {'GAME_BSS': 0}
    for owner in manifest['regions']:
        for symbol, binding in owner.get('build', {}).get('bindings', {}).items():
            if (binding.get('coordinate') != 'DGROUP_offset' or symbol in explicit_publics
                    or symbol.lower() in explicit_casefold):
                continue
            if 'offset' in binding:
                offset = binding['offset']
            elif binding.get('owner') in owners:
                offset = (owners[binding['owner']]['start'] - 512 - frame
                          + binding.get('addend', 0))
            else:
                continue
            if 0 <= offset < initialized_span:
                add_alias(symbol, offset)
            elif initialized_span <= offset < initialized_span + bss_source['length']:
                bss_offset = offset - initialized_span
                if symbol in bss_public_offsets and bss_public_offsets[symbol] != bss_offset:
                    raise ValueError('Conflicting recovered BSS public offset')
                bss_public_offsets[symbol] = bss_offset
    if source_bss_publics != bss_public_offsets:
        raise ValueError('Canonical BSS public map differs from linker-binding evidence')
    planned_bss_publics = {}
    for contribution in bss_contributions:
        for symbol, offset in contribution['publics'].items():
            absolute = contribution['logical_start'] + offset
            if symbol in planned_bss_publics and planned_bss_publics[symbol] != absolute:
                raise ValueError('Conflicting BSS contribution public')
            planned_bss_publics[symbol] = absolute
    if planned_bss_publics != source_bss_publics:
        raise ValueError('BSS contribution plan differs from canonical public map')
    separated = []
    for owner_id, entry in staged.items():
        module = OmfReader().read((work / 'WORK' / entry['object']).read_bytes())
        if entry['kind'] == 'owner' and module.segment_length('_DATA') and owner_id not in retained:
            raise ValueError(f'{owner_id}: compiler DATA must have native source ownership')
    if startup_aliases or runtime_aliases:
        raise ValueError('Production DATA requires natural runtime/startup publics')
    for contribution in bss_contributions:
        asm = bss_asm_source(contribution['length'], contribution['publics'],
                             contribution.get('typed_reserves', ()))
        asm_path = work / 'WORK' / contribution['assembly']
        contribution['asm_path'] = asm_path
        contribution['asm_sha256'] = sha(asm.encode())
        asm_path.write_bytes(asm.encode('ascii'))
        # TASM records the source mtime in a COMENT record. Pin it so identical
        # canonical source produces an identical relocatable object on every run.
        os.utime(asm_path, (315532800, 315532800))
    names, sources = [], []
    for index, part in enumerate(parts):
        if part['spec'].get('retain_in_code_object'):
            native = OmfReader().read((work / 'WORK' / member_objects[part['spec']['code_owner']]['object']).read_bytes())
            native_publics = {p['name']: p['offset'] for p in native.publics_in('_DATA')}
            aliases = {k:v for k,v in part['publics'].items() if k != part['spec']['id']}
            if any(native_publics.get(k) != v + part['native_offset'] for k,v in aliases.items()):
                raise ValueError(f"Retained DATA {part['spec']['id']} requires unavailable native aliases: {aliases}; native={native_publics}")
            continue
        name = f'D{index:04}.OBJ'
        blob = emit_data(part['data'], part['publics'], part['refs'], f'D{index:04}')
        (work / 'WORK' / name).write_bytes(blob)
        names.append('C:\\WORK\\' + name)
        sources.append({'owner': part['spec']['id'], 'bytes': len(part['data']),
                        'format': part['spec']['format'], 'pointer_fixups': len(part['refs']),
                        'object_sha256': sha(blob)})
    return parts, bss_contributions, sources, separated
