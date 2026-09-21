"""Test a whole-translation-unit hypothesis over a contiguous run of production modules.

Usage:
  python tools/probe_tu.py FIRST_MODULE LAST_MODULE [--flags "-B"] [--source OWNER=alt.C ...]
  python tools/probe_tu.py --list FIRST LAST        # only show the run and its flags

FIRST/LAST name production-plan module ids (F_695E, C_6C26_6C87, ...) or any member;
every module between them in object_order is compiled from its production source
(or an override) as ONE C unit with the pinned flags plus --flags, and the emitted
_TEXT extent is bound and compared against the original bytes of the whole run.
Unlike probe_module_group.py this does not require the members to share their
individually established flags_append: a unit that contains inline asm goes through
TASM (-B) as a whole, so a run of separately matched modules with mixed ''/-B
flags can still be one historical TU.  An assembler module inside the run must be
given a C candidate (--source MODULE=build/probes/x.C, typically the same functions
written as C functions with asm bodies) which is compiled as part of the unit.
Research only: nothing is edited.
"""
import argparse
from pathlib import Path
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))

from mz import MZ
from reconstruct import bind_region, compile_sources, owned_library_modules, read_json, read_object


def module_run(plan, first, last):
    modules = plan['modules']
    by_member = {m: module['id'] for module in modules for m in module.get('members', [])}
    ids = [m['id'] for m in modules]
    a = ids.index(by_member.get(first, first))
    b = ids.index(by_member.get(last, last))
    if a > b:
        a, b = b, a
    return modules[a:b + 1]


def probe(first, last, flags='', overrides=None, root=ROOT, hex_dump=False, asm_unit=None):
    manifest = read_json(root / 'layout/manifest.json')
    lock = read_json(root / 'layout/toolchain.json')
    original = (root / 'assets/AEPROG.EXE').read_bytes()
    regions = {r['id']: r for r in manifest['regions']}
    plan = read_json(root / 'layout/production-plan.json')
    run = module_run(plan, first, last)
    # An assembler module inside the run must be replaced by a C candidate
    # (--source MODULE=path.C); the candidate is then compiled as part of the unit.
    asm = [m['id'] for m in run if m['tool'] != 'TCC.EXE' and m['id'] not in (overrides or {})]
    if asm and not asm_unit:
        raise SystemExit(f'run contains assembler modules {asm}; give each a C candidate with --source MODULE=path.C')
    for a, b in zip(run, run[1:]):
        if a['end'] != b['start']:
            raise SystemExit(f"{a['id']} ends at {a['end']:#x} but {b['id']} starts at {b['start']:#x}: not contiguous")
    sources, seen = [], set()
    for m in run:
        for s in (m.get('sources') or [m['source']]):
            path = s['path'] if isinstance(s, dict) else s
            path = (overrides or {}).get(m['id'], path)
            if path not in seen:
                seen.add(path)
                sources.append(path)
    concat = root / 'build/probes/_tu'
    concat.mkdir(parents=True, exist_ok=True)
    unit_id = f"TU_{run[0]['id']}_{run[-1]['id']}"
    staged = concat / (unit_id + '.C')
    staged.write_bytes(b'\r\n'.join((root / src).read_bytes() for src in sources))
    bindings, publics = {}, []
    for m in run:
        for symbol, binding in m['build'].get('bindings', {}).items():
            bindings.setdefault(symbol, binding)
        for member in m.get('members', []):
            for symbol, binding in regions.get(member, {}).get('build', {}).get('bindings', {}).items():
                bindings.setdefault(symbol, binding)
        publics.extend(m.get('publics', []))
    # Private initialized DATA of the members: one unit emits one _DATA segment in
    # source order, so the members' DATA components must be contiguous in DGROUP.
    data_owners = sorted((regions[d] for m in run for d in m.get('data_owners', [])), key=lambda o: o['start'])
    module_segments = {}
    if data_owners:
        for a, b in zip(data_owners, data_owners[1:]):
            if a['end'] != b['start']:
                raise SystemExit(f"DATA components {a['id']} and {b['id']} are not contiguous in DGROUP: "
                                 f"the run cannot be one unit")
        module_segments['_DATA'] = {'owner': data_owners[0]['id'], 'coordinate': 'DGROUP_offset', 'addend': 0}
    # Code symbols the members never bound (calls that used to be relative
    # displacements or lived inside one of the merged modules) resolve through
    # the public index; the promotion must then declare them as plan bindings.
    index = {p['symbol']: p for p in read_json(root / 'layout/public-index.json')['publics']
             if p['classification'] == 'KNOWN_FUNCTION_ENTRY'}
    owner = {'id': unit_id, 'start': run[0]['start'], 'end': run[-1]['end'], 'kind': 'MATCHING_C',
             'source': staged.relative_to(root).as_posix(), 'members': [x for m in run for x in m['members']],
             'build': {'segment': '_TEXT', 'public': run[0]['build']['public'], 'span': max(1, len(publics)),
                       'flags_append': flags, 'bindings': bindings, 'module_segments': module_segments}}
    if asm_unit:
        # One hand-written TASM module hypothesis: assemble the candidate as the whole run.
        owner.update(kind='MATCHING_ASM', source=Path(asm_unit).resolve().relative_to(root).as_posix())
    with tempfile.TemporaryDirectory(dir=root / 'build') as temporary:
        work = Path(temporary)
        try:
            receipts, _ = compile_sources(root, [owner], work, root / 'toolchain', None, lock)
        except ValueError as error:
            saved = root / 'build/probe-tu-build.log'
            for log in (work / 'host.log', work / 'WORK/BUILD.LOG'):
                if log.exists():
                    saved.write_bytes(log.read_bytes())
                    break
            raise SystemExit(f'{error}; compiler log copied to {saved}')
        module = read_object((work / receipts[unit_id]['object']).read_bytes())
        library = owned_library_modules(manifest['regions'], root / 'toolchain', lock)
        mz = MZ.parse(original)
        expected = original[owner['start']:owner['end']]
        emitted = module.publics_in('_TEXT')
        print(f"{unit_id}: {len(run)} modules, " + (f"one TASM unit {owner['source']}" if asm_unit else f"{len(sources)} sources, flags '{flags}'"))
        wrong = []
        for m in run:
            for p in m.get('publics', []):
                got = next((e['offset'] for e in emitted if e['name'] == p['symbol']), None)
                want = m['start'] - owner['start'] + p['offset']
                if got != want:
                    wrong.append((p['symbol'], want, got))
        if module.segment_length('_TEXT') != len(expected):
            print(f"  LENGTH {module.segment_length('_TEXT')} != {len(expected)}")
        for symbol, want, got in wrong:
            print(f"  public {symbol}: expected +0x{want:X}, emitted {got if got is None else f'+0x{got:X}'}")
        if data_owners:
            emitted_data = module.segment_bytes('_DATA')
            wanted = original[data_owners[0]['start']:data_owners[0]['start'] + len(emitted_data)]
            relocated = {f['offset'] + i for f in module.fixups_in('_DATA') for i in range(f['width'])}
            bad = next((i for i, (a, b) in enumerate(zip(wanted, emitted_data)) if a != b and i not in relocated), None)
            expected_data = data_owners[-1]['end'] - data_owners[0]['start']
            print(f"  _DATA: {len(emitted_data)} bytes ({len(relocated)} fixup bytes) "
                  + ('EXACT' if bad is None and len(emitted_data) == expected_data
                     else f'MISMATCH at +0x{bad:X}' if bad is not None else f'LENGTH {len(emitted_data)} != {expected_data}'))
        added = []
        for f in module.fixups_in('_TEXT'):
            symbol = f['target']
            if f['target_kind'] == 'external' and symbol not in bindings and symbol in index:
                entry = index[symbol]
                region = next((r for r in regions.values() if r['start'] - 512 == entry['address']), None)
                if region:
                    bindings[symbol] = {'coordinate': 'code_offset', 'owner': region['id'], 'public': symbol,
                                        'addend': 0, 'evidence': f"functions/{region['id']}.entry"}
                    added.append(f"{symbol}={region['id']}")
        if added:
            print('  bindings resolved through the public index (declare them at promotion): ' + ', '.join(sorted(set(added))))
        try:
            data, proof = bind_region(owner, module, mz, manifest['frames'], manifest['regions'], library)
            masked = set()
        except ValueError as error:
            print(f'  BIND ERROR: {error}')
            # Raw object bytes hold unresolved fixup fields; compare only the rest.
            data = module.segment_bytes('_TEXT')
            proof = {'fixups': []}
            masked = {f['offset'] + i for f in module.fixups_in('_TEXT') for i in range(f['width'])}
        first_bad = next((i for i, (a, b) in enumerate(zip(expected, data)) if a != b and i not in masked), None)
        if first_bad is None and len(expected) == len(data) and not wrong and not masked:
            print(f"  EXACT ({len(data)} bytes, {len(proof['fixups'])} fixups)")
            status = 0
        elif first_bad is None and len(expected) == len(data) and not wrong:
            print(f"  bytes agree outside fixup fields but the unit is NOT bound ({len(masked)} fixup bytes unchecked)")
            status = 1
        else:
            if first_bad is not None:
                owner_at = next((m['id'] for m in run if m['start'] <= owner['start'] + first_bad < m['end']), '?')
                print(f"  MISMATCH at +0x{first_bad:X} (file 0x{owner['start']+first_bad:06X}, in {owner_at}): "
                      f"original={expected[first_bad]:02X} reconstructed={data[first_bad]:02X}"
                      + (' (fixup bytes masked)' if masked else ''))
            status = 1
        if hex_dump or status:
            print('  reconstructed:', data.hex())
            print('  original:     ', expected.hex())
        return status


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('first')
    parser.add_argument('last')
    parser.add_argument('--flags', default='', help='extra compiler flags for the whole unit (e.g. -B, -k)')
    parser.add_argument('--source', action='append', default=[], metavar='MODULE=PATH')
    parser.add_argument('--list', action='store_true', help='only list the run')
    parser.add_argument('--hex', action='store_true')
    parser.add_argument('--asm', metavar='UNIT.ASM', help='assemble this one TASM module as the whole run instead of compiling C')
    args = parser.parse_args(argv)
    if args.list:
        plan = read_json(ROOT / 'layout/production-plan.json')
        for m in module_run(plan, args.first, args.last):
            src = m.get('source') or ','.join(s['path'] for s in m['sources'])
            print(f"{m['id']:14} {m['start']:#7x}-{m['end']:#7x} {m['tool']:8} {' '.join(m['flags']):26} {src}")
        return 0
    return probe(args.first, args.last, args.flags, dict(i.split('=', 1) for i in args.source), hex_dump=args.hex,
                 asm_unit=args.asm)


if __name__ == '__main__':
    sys.exit(main())
