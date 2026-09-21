"""Generate the complete canonical module plan, without compiling or linking."""
import copy
from reconstruct import ROOT, read_json, write_json
from canonical_link_plan import order_data_modules

SHARED = ('C_01BC_0355', 'C_200F_2A70', 'C_31C4_3986', 'C_49E3_4A93', 'C_5321_56C6', 'C_5A3B_6021',
          'C_622C_625D', 'C_6266_68AA', 'C_695E_697D', 'C_6990_6997', 'C_6C26_6C87', 'C_6FC3_747B',
          'C_75F3_7856', 'C_7D91_880A', 'C_8A37_969D', 'C_9962_99E2', 'C_984C_9871', 'C_9A0E_9D79',
          'C_A09D_A24E', 'C_A33F_AD0E', 'C_AD25_AF45', 'C_AF45_C15E', 'C_CDDD_D344', 'C_D85F_D8F0',
          'C_D99B_DA49', 'C_DB60_DDC7', 'C_E095_E54D')


def generate(root=ROOT):
    manifest = read_json(root / 'layout/manifest.json')
    owners = {o['id']: o for o in manifest['regions']}
    modules = copy.deepcopy(read_json(root / 'layout/structural-source-modules.json')['modules'])
    for name in SHARED:
        recipe = read_json(root / f'recipes/modules/{name}.json')
        members = [s['owner'] for s in recipe['sources']]
        module = copy.deepcopy(owners[members[0]])
        module.update(id=recipe['id'], members=members, sources=list(dict.fromkeys(s['path'] for s in recipe['sources'])),
                      end=owners[members[-1]]['end'], recipe=f'recipes/modules/{name}.json')
        module.pop('source')
        modules.append(module)
    members = {m for module in modules for m in module['members']}
    modules += [copy.deepcopy(o) for o in owners.values()
                if o['kind'] in ('MATCHING_C', 'MATCHING_ASM')
                and o['start'] >= 512 + 0x1bc and o['id'] not in members and o['id'] != 'F_F9BE']
    modules.sort(key=lambda m: m['start'])
    for index, module in enumerate(modules):
        module['object'] = f'R{index:04}.OBJ'
        module.setdefault('members', [module['id']])
        module['tool'] = 'TCC.EXE' if module['kind'] == 'MATCHING_C' else 'TASM.EXE'
        module['flags'] = ((read_json(root / 'layout/toolchain.json')['flags'] + ' ' +
                            module['build'].get('flags_append', '')).split()
                           if module['kind'] == 'MATCHING_C' else ['/mx'])
        module['publics'] = [{'owner': m, 'symbol': owners[m].get('build', {}).get('public'),
                             'offset': owners[m]['start'] - module['start']}
                            for m in module['members'] if m in owners]
        data_owners = sorted([o for o in owners.values() if o.get('build', {}).get('code_owner')
                              in module['members']], key=lambda o: o['start'])
        module['data_component_offsets'] = {o['id']: o['start'] - data_owners[0]['start'] for o in data_owners}
        module['data_offsets'] = {}
        for o in data_owners:
            module['data_offsets'].setdefault(o['build']['code_owner'], o['start'] - data_owners[0]['start'])
        if data_owners:
            module['build']['module_segments']['_DATA'] = {'owner': data_owners[0]['id'],
                                                          'coordinate': 'DGROUP_offset', 'addend': 0}
        module['data_owners'] = [o['id'] for o in data_owners]
    occupied = {m for module in modules for m in module['members']}
    pads = [dict(o, object=f'P{i:04}.OBJ') for i, o in enumerate(
        o for o in owners.values() if o.get('classification') == 'alignment_padding'
        and 512 + 0x1bc <= o['start'] < max(m['end'] for m in modules) and o['id'] not in occupied)]
    linker_alignment = []
    for pad in pads:
        target = pad.get('build', {}).get('linker_alignment_before')
        if target:
            module = next((m for m in modules if m['id'] == target), None)
            if module is None or module['start'] != pad['end'] or pad['end'] - pad['start'] != 1 or (pad['end'] - 512) % 2:
                raise ValueError('Invalid natural word-alignment ownership')
            linker_alignment.append({'owner': pad['id'], 'before_module': target, 'alignment': 'word'})
    pads = [p for p in pads if not p.get('build', {}).get('linker_alignment_before')]
    ordered = sorted(modules + pads, key=lambda m: m['start'])
    data = [dict(s, object=f'D{i:04}.OBJ') for i, s in enumerate(
        read_json(root / 'recipes/data/game-initialized.json')['components'])]
    bss = read_json(root / 'recipes/data/bss-contributions.json')['contributions']
    names = [m['object'] for m in ordered if not m.get('build', {}).get('linker_library_module')] + [d['object'] for d in data] + [b['object'] for b in bss]
    anchors = {member: m['object'] for m in modules for member in m['members']}
    names, placements = order_data_modules(names, [(d['id'], d['object']) for d in data], anchors,
                                            read_json(root / 'recipes/data/canonical-link-plan.json'))
    # Retained DATA travels with its untouched compiler object. Only remove the
    # duplicate DATA object; never move code to force a historical address.
    retained = {}
    for component in data:
        if component.get('retain_in_code_object'):
            if component['format'] != 'compiled-data':
                raise ValueError('Only compiled DATA can remain in its code object')
            native = anchors[component['code_owner']]
            retained[component['object']] = native
    names = [name for name in names if name not in retained]
    expected = []
    for d in data:
        name = retained.get(d['object'], d['object'])
        if not expected or name != expected[-1]:
            expected.append(name)
    if len(expected) != len(set(expected)):
        raise ValueError('Retained module DATA components are not contiguous')
    if [name for name in names if name in set(expected)] != expected:
        raise ValueError('Natural retained DATA order differs from source order')
    return {'format': 'empires-production-plan-v1', 'modules': modules, 'padding': pads, 'linker_alignment': linker_alignment,
            'data': data, 'bss': bss, 'object_order': ['C0C.OBJ'] + names,
            'library': 'CC.LIB', 'linker': 'TLINK.EXE', 'link_flags': ['/s'],
            'data_placements': placements}


def checked_plan(root=ROOT):
    plan = read_json(root / 'layout/production-plan.json')
    if plan != generate(root):
        raise ValueError('Module plan is stale: python tools/production_plan.py')
    return plan


if __name__ == '__main__':
    plan = generate()
    write_json(ROOT / 'layout/production-plan.json', plan)
    print(f"Plan: {len(plan['modules'])} source modules; {len(plan['object_order'])} objects; one TLINK")
