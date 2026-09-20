"""Conservative recursive 16-bit CFG. Never linearly sweep unvisited bytes."""
from collections import defaultdict, deque
import sys
from reconstruct import ROOT, read_json
sys.path.insert(0, str(ROOT / 'build/python-deps'))
try:
    import capstone as cs
    from capstone.x86 import X86_OP_IMM, X86_OP_MEM, X86_REG_CS
except ImportError as exc:
    raise SystemExit('Install decoder: python -m pip install --no-user --target build/python-deps -r requirements-factory.txt') from exc
if cs.__version__ != '5.0.3':
    raise SystemExit('Factory decoder must be Capstone 5.0.3; install requirements-factory.txt')


def target_index(root=ROOT):
    manifest = read_json(root / 'layout/manifest.json')
    owners = manifest['regions']
    entries = defaultdict(list)
    for owner in owners:
        if owner.get('build', {}).get('segment') == '_TEXT':
            name = owner['build'].get('public')
            if name:
                entries[owner['start'] - 512].append({'symbol': name, 'classification': 'KNOWN_FUNCTION_ENTRY', 'owner': owner['id']})
    # Fresh production object's secondary publics are recorded in the canonical index.
    path = root / 'layout/public-index.json'
    if path.exists():
        for p in read_json(path)['publics']:
            entries[p['address']].append(p)
    modules = read_json(root / 'layout/structural-source-modules.json')['modules']
    owners = owners + [dict(m, structural_module=True) for m in modules]
    return entries, owners, manifest['frames']['DGROUP']


def resolve_target(address, entries, owners, local_range=None, blocks=()):
    exact = entries.get(address, [])
    if exact:
        order = {'EXACT_PUBLIC': 0, 'KNOWN_FUNCTION_ENTRY': 1, 'KNOWN_SECONDARY_PUBLIC': 2}
        selected = sorted(exact, key=lambda x: (order.get(x['classification'], 3), x['symbol']))[0]
        return {'address': address, **selected, 'aliases': sorted({x['symbol'] for x in exact})}
    owner = next((o for o in owners if o['start'] - 512 <= address < o['end'] - 512), None)
    if owner and address == owner['start'] - 512:
        return {'address': address, 'classification': 'KNOWN_OWNER_BOUNDARY', 'owner': owner['id']}
    structural = next((o for o in owners if o.get('structural_module') and o['start']-512 <= address < o['end']-512), None)
    if structural:
        return {'address':address,'classification':'KNOWN_STRUCTURAL_MODULE','module':structural['id'],
                'members':structural['members'],'module_offset':address-(structural['start']-512)}
    if address in blocks:
        return {'address': address, 'classification': 'LOCAL_BASIC_BLOCK'}
    if owner:
        return {'address': address, 'classification': 'UNKNOWN_INTERIOR_ADDRESS', 'owner': owner['id'],
                'owner_offset': address - (owner['start'] - 512)}
    return {'address': address, 'classification': 'UNKNOWN_ADDRESS'}


def recursive_cfg(binary, roots, base=0, entries=None, owners=(), data_ranges=(), indirect_targets=None):
    entries = entries or {}
    indirect_targets = indirect_targets or {}
    decoder = cs.Cs(cs.CS_ARCH_X86, cs.CS_MODE_16)
    decoder.detail = True
    pending = deque(sorted(set(r['offset'] for r in roots)))
    instructions, edges, issues, indirect, tables = {}, [], [], [], []
    occupied = {}
    def is_data(offset):
        return any(d['start'] <= offset < d['end'] for d in data_ranges)
    def enqueue(source, target, kind):
        resolution = resolve_target(base + target, entries, owners, (base, base + len(binary)))
        edges.append({'source': source, 'target': target, 'kind': kind, 'resolution': resolution})
        if 0 <= target < len(binary):
            pending.append(target)
    while pending:
        offset = pending.popleft()
        if offset in instructions or not 0 <= offset < len(binary):
            continue
        if is_data(offset):
            issues.append({'offset': offset, 'kind': 'CONTROL_FLOW_ENTERS_DATA'})
            continue
        if offset in occupied:
            issues.append({'offset': offset, 'kind': 'OVERLAPPING_INSTRUCTION', 'instruction': occupied[offset]})
            continue
        ins = next(decoder.disasm(binary[offset:offset + 15], base + offset, count=1), None)
        if ins is None or any(is_data(x) or x in occupied for x in range(offset, offset + (ins.size if ins else 1))):
            issues.append({'offset': offset, 'kind': 'UNDECODABLE_OR_OVERLAPPING'})
            continue
        jump = ins.group(cs.CS_GRP_JUMP) or ins.mnemonic in ('loop','loope','loopne','jcxz','jecxz')
        call = ins.group(cs.CS_GRP_CALL)
        terminal = ins.group(cs.CS_GRP_RET) or ins.group(cs.CS_GRP_IRET) or ins.mnemonic in ('hlt', 'int3', 'ud2')
        row = {'offset': offset, 'size': ins.size, 'bytes': ins.bytes.hex(), 'mnemonic': ins.mnemonic,
               'operands': ins.op_str, 'flow': 'RETURN' if terminal else 'CALL' if call else 'BRANCH' if jump else 'FALLTHROUGH'}
        row['memory_references'] = [{'segment':ins.reg_name(o.mem.segment) or ('ss' if ins.reg_name(o.mem.base)=='bp' else 'ds'),
                                     'base':ins.reg_name(o.mem.base), 'index':ins.reg_name(o.mem.index),
                                     'displacement':o.mem.disp,'width':o.size}
                                    for o in ins.operands if o.type == X86_OP_MEM]
        instructions[offset] = row
        for x in range(offset, offset + ins.size): occupied[x] = offset
        if jump or call:
            if len(ins.operands) == 1 and ins.operands[0].type == X86_OP_IMM:
                target = (ins.operands[0].imm & 0xffff) - base
                proof = indirect_targets.get(str(offset), {})
                if isinstance(proof, dict) and proof.get('mutable_direct'):
                    indirect.append({'offset':offset, 'mnemonic':ins.mnemonic, 'operands':ins.op_str, **proof})
                    for target in proof['targets']: enqueue(offset, target, 'INDIRECT_KNOWN')
                else:
                    enqueue(offset, target, 'CALL' if call else 'BRANCH')
            else:
                proof = indirect_targets.get(str(offset), [])
                known = proof.get('targets', []) if isinstance(proof, dict) else proof
                indirect.append({'offset': offset, 'mnemonic': ins.mnemonic, 'operands': ins.op_str,
                                 'targets': known, 'complete': proof.get('complete',False) if isinstance(proof,dict) else bool(known),
                                 'evidence':proof if isinstance(proof,dict) else None})
                for target in known: enqueue(offset, target, 'INDIRECT_KNOWN')
                for operand in ins.operands:
                    if operand.type == X86_OP_MEM and operand.mem.segment == X86_REG_CS:
                        start = operand.mem.disp - base
                        if 0 <= start < len(binary):
                            tables.append({'start': start, 'end': None, 'referenced_by': offset,
                                           'classification': 'PROBABLE_TABLE', 'confidence': 'LOW',
                                           'reason': 'CS-relative indirect memory operand; extent and target set require evidence'})
        if not terminal and ins.mnemonic not in ('jmp', 'ljmp'):
            # INTO and BIOS/DOS interrupts can return; retain fallthrough conservatively.
            enqueue(offset, offset + ins.size, 'FALLTHROUGH')
    leaders = {r['offset'] for r in roots} | {e['target'] for e in edges if e['kind'] != 'FALLTHROUGH'}
    leaders |= {i['offset'] + i['size'] for i in instructions.values() if i['flow'] != 'FALLTHROUGH'}
    blocks = []
    current = None
    for offset, ins in sorted(instructions.items()):
        if current is None or offset in leaders or current['end'] != offset:
            current = {'start': offset, 'end': offset, 'instructions': [], 'roots': []}
            blocks.append(current)
        current['instructions'].append(offset)
        current['end'] = offset + ins['size']
        if ins['flow'] != 'FALLTHROUGH': current = None
    block_addresses = {base + b['start'] for b in blocks}
    for edge in edges:
        edge['resolution'] = resolve_target(base + edge['target'], entries, owners, blocks=block_addresses)
    adjacency = defaultdict(list)
    for edge in edges:
        if edge['target'] in instructions: adjacency[edge['source']].append(edge['target'])
    reach = defaultdict(set)
    for root in roots:
        todo, visited = [root['offset']], set()
        while todo:
            at = todo.pop()
            if at in visited or at not in instructions: continue
            visited.add(at); reach[at].update(root['names']); todo.extend(adjacency[at])
    for block in blocks:
        block['roots'] = sorted(set().union(*(reach[x] for x in block['instructions'])))
    gaps, begin = [], None
    for offset in range(len(binary) + 1):
        if offset < len(binary) and offset not in occupied:
            if begin is None: begin = offset
        elif begin is not None:
            gaps.append({'start': begin, 'end': offset, 'classification': 'UNREACHABLE_UNKNOWN'})
            begin = None
    return {'format': 'empires-runtime-cfg-v2', 'decoder': {'name':'Capstone','version':cs.__version__,'mode':16}, 'base_load_address': base, 'bytes': len(binary),
            'roots': roots, 'instructions': list(sorted(instructions.values(), key=lambda i:i['offset'])),
            'basic_blocks': blocks, 'edges': sorted(edges, key=lambda e:(e['source'], e['kind'], e['target'])),
            'direct_calls': [e for e in edges if e['kind'] == 'CALL'],
            'resolved_external_targets':[e for e in edges if not 0<=e['target']<len(binary) and e['resolution']['classification'] not in ('UNKNOWN_ADDRESS','UNKNOWN_INTERIOR_ADDRESS')],
            'local_targets':sorted({e['target'] for e in edges if e['kind']!='FALLTHROUGH' and 0<=e['target']<len(binary)}), 'indirect_sites': indirect,
            'probable_tables': sorted(tables, key=lambda t:(t['start'],t['referenced_by'])),
            'unreachable_gaps': gaps, 'issues': sorted(issues,key=lambda i:(i['offset'],i['kind']))}


def runtime_cfg(binary, root=ROOT):
    spec = read_json(root / 'recipes/runtime/oracle.json')
    entries, owners, _ = target_index(root)
    owner = next(o for o in owners if o['id'] == 'RUNTIME_BLOCK')
    base = owner['start'] - 512
    aliases = defaultdict(list)
    for public in spec['publics']:
        if public['offset'] < len(binary):
            aliases[public['offset']].append(public['name'])
            entries[base + public['offset']].append({'symbol': public['name'], 'classification': 'EXACT_PUBLIC', 'owner':'RUNTIME_BLOCK'})
    roots = [{'offset': offset, 'names': sorted(names)} for offset, names in sorted(aliases.items())]
    roots += spec['extra_roots']
    from runtime_boundaries import recover
    tables, targets, regions = recover(binary)
    if spec['data_ranges'] != tables or spec['indirect_targets'] != targets:
        raise ValueError('Runtime boundary recipe differs from byte-pinned consumer evidence')
    result = recursive_cfg(binary, roots, base, entries, owners, tables, {**spec['indirect_targets'], **targets})
    for region in regions:
        a,b=region['range']
        region['direct_branches']=[e for e in result['edges'] if a<=e['source']<b and e['kind'] in ('CALL','BRANCH')]
        region['indirect_branches']=[s for s in result['indirect_sites'] if a<=s['offset']<b]
        region['exits']=[e for e in result['edges'] if a<=e['source']<b and not a<=e['target']<b]
        region['returns']=[i['offset'] for i in result['instructions'] if a<=i['offset']<b and i['flow']=='RETURN']
    gaps=[]
    for gap in result['unreachable_gaps']:
        cuts=sorted({gap['start'],gap['end']}|{v for t in tables for v in (t['start'],t['end']) if gap['start']<v<gap['end']})
        for a,b in zip(cuts,cuts[1:]):
            data=next((t for t in tables if t['start']<=a<b<=t['end']),None)
            gaps.append({'start':a,'end':b,'classification':data['classification'] if data else 'UNREACHABLE_UNKNOWN'})
    result['unreachable_gaps']=gaps
    result['typed_tables'] = tables
    result['boundary_regions'] = regions
    result['proven_indirect_roots'] = sorted({t for p in targets.values() for t in p['targets']})
    return result
