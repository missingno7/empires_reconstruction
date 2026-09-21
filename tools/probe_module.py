"""Compile one or more production source modules and compare each against the original bytes.

Usage:
  python tools/probe_module.py F_5321 [F_5382 ...] [--source F_5321=path/to/alt.C ...]

Each owner is compiled with the pinned toolchain exactly as the production build
does, bound from its own compiler object and declared metadata only, then compared
with the original region. This is a research probe: it never edits anything and
never replaces full EXE acceptance. Alternative sources must lie inside the project
tree (for example build/probes/F_5321.C).
"""
import argparse
from pathlib import Path
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))

from mz import MZ
from reconstruct import (bind_region, compile_sources, owned_library_modules,
                         read_json, read_object)


def probe(owner_ids, overrides=None, root=ROOT, as_c=None):
    manifest = read_json(root / 'layout/manifest.json')
    lock = read_json(root / 'layout/toolchain.json')
    original = (root / 'assets/AEPROG.EXE').read_bytes()
    regions = {r['id']: r for r in manifest['regions']}
    # The production plan is the consumed build description; its module entries
    # carry the current bindings and data-segment placement. Manifest regions
    # only supply owners the plan does not list.
    plan = {m['id']: m for m in read_json(root / 'layout/production-plan.json')['modules']}
    owners, compiled, seen = [], [], set()
    members = {m: module['id'] for module in plan.values() for m in module.get('members', []) if module.get('sources')}
    concat = root / 'build/probes/_shared'
    for owner_id in owner_ids:
        override = overrides.get(owner_id) if overrides else None
        if owner_id in members or (owner_id in plan and plan[owner_id].get('sources')):
            # A member of a shared multi-source module is compiled as the whole module,
            # exactly as the production build concatenates its sources.
            module = plan[members.get(owner_id, owner_id)]
            # Every override naming a member of this module applies to the same concatenation.
            member_overrides = {regions[m]['source']: overrides[m] for m in module['members']
                                if overrides and m in overrides and m in regions}
            if override and owner_id in plan:
                member_overrides = {s: override for s in module['sources']} if len(module['sources']) == 1 else member_overrides
            sources = [member_overrides.get(s, s) for s in module['sources']]
            concat.mkdir(parents=True, exist_ok=True)
            staged = concat / (module['id'] + '.C')
            staged.write_bytes(b'\r\n'.join((root / src).read_bytes() for src in sources))
            owner_id, override = module['id'], staged.relative_to(root).as_posix()
        if owner_id in seen:
            continue
        seen.add(owner_id)
        if owner_id not in plan and owner_id not in regions:
            raise SystemExit(f'unknown owner {owner_id}')
        owner = dict(plan.get(owner_id) or regions[owner_id])
        if as_c and owner_id in as_c:
            # Probe a C candidate for a region that production still assembles:
            # compile the override with the pinned compiler flags and bind it as C.
            owner['kind'] = 'MATCHING_C'
            owner['build'] = {**owner['build'], 'flags_append': as_c[owner_id]}
        if owner.get('sources'):
            # Whole-module comparison: the production build checks the full compiled extent.
            bindings = dict(owner['build'].get('bindings', {}))
            for member in owner.get('members', []):
                for symbol, binding in regions.get(member, {}).get('build', {}).get('bindings', {}).items():
                    bindings.setdefault(symbol, binding)
            owner['build'] = {**owner['build'], 'span': max(1, len(owner.get('publics', []))), 'bindings': bindings}
        owners.append(owner)
        # Only the compiler input is overridden; binding keeps the plan owner so
        # that shared data components (matched by source path) still resolve.
        compiled.append({**owner, 'source': override} if override else owner)
    (root / 'build').mkdir(exist_ok=True)
    results = {}
    with tempfile.TemporaryDirectory(dir=root / 'build') as temporary:
        work = Path(temporary)
        try:
            receipts, _ = compile_sources(root, compiled, work, root / 'toolchain', None, lock)
        except ValueError as error:
            log = work / 'WORK/BUILD.LOG'
            saved = root / 'build/probe-build.log'
            if log.exists():
                saved.write_bytes(log.read_bytes())
            raise SystemExit(f'{error}; compiler log copied to {saved}')
        library = owned_library_modules(manifest['regions'], root / 'toolchain', lock)
        mz = MZ.parse(original)
        for owner, unit in zip(owners, compiled):
            module = read_object((work / receipts[owner['id']]['object']).read_bytes())
            expected = original[owner['start']:owner['end']]
            data_status = []
            if owner['id'] not in plan:
                data_status.append('manifest-only owner: data placement not checked')
            for segment, place in (owner['build'].get('module_segments', {}) if owner['id'] in plan else {}).items():
                emitted = module.segment_bytes(segment)
                if 'owner' in place:
                    start = regions[place['owner']]['start'] + place.get('addend', 0)
                elif place.get('coordinate') == 'DGROUP_offset' and 'offset' in place:
                    start = 512 + manifest['frames']['DGROUP'] + place['offset']
                else:
                    data_status.append(f'{segment}: placement not checked ({place})'); continue
                wanted = original[start:start + len(emitted)]
                # Raw object bytes hold unresolved fixup fields; compare only the rest.
                relocated = {f['offset'] + i for f in module.fixups_in(segment) for i in range(f['width'])}
                bad = next((i for i, (a, b) in enumerate(zip(wanted, emitted)) if a != b and i not in relocated), None)
                data_status.append(f'{segment}:{len(emitted)} bytes {len(relocated)} fixup bytes ' + ('EXACT' if bad is None else f'MISMATCH at +0x{bad:X}'))
            try:
                data, proof = bind_region(owner, module, mz, manifest['frames'], manifest['regions'], library)
            except ValueError as error:
                data, proof = module.segment_bytes(owner['build']['segment']), {'fixups': []}
                results[owner['id']] = {'status': f'BIND ERROR: {error}', 'bytes': len(data), 'expected': len(expected),
                                        'fixups': 0, 'source': unit['source'], 'data_segments': data_status,
                                        'reconstructed': data.hex(), 'original': expected.hex()}
                continue
            first = next((i for i, (a, b) in enumerate(zip(expected, data)) if a != b), None)
            if first is None and len(expected) == len(data):
                status = 'EXACT'
            elif first is None:
                status = f'LENGTH {len(data)} != {len(expected)}'
                first = min(len(expected), len(data))
            else:
                status = f'MISMATCH at +0x{first:X} (file 0x{owner["start"]+first:06X}): original={expected[first]:02X} reconstructed={data[first]:02X}'
            if data_status and any('MISMATCH' in d for d in data_status): status = 'DATA ' + ', '.join(data_status)
            results[owner['id']] = {'status': status, 'bytes': len(data), 'expected': len(expected),
                                    'fixups': len(proof['fixups']), 'source': unit['source'],
                                    'data_segments': data_status,
                                    'reconstructed': data.hex(), 'original': expected.hex()}
    return results


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('owners', nargs='+')
    parser.add_argument('--source', action='append', default=[], metavar='OWNER=PATH')
    parser.add_argument('--hex', action='store_true', help='print reconstructed and original bytes')
    parser.add_argument('--as-c', action='append', default=[], metavar='OWNER[=FLAGS]',
                        help='treat OWNER as a C unit (optional extra compiler flags such as -B) for this probe')
    args = parser.parse_args(argv)
    overrides = dict(item.split('=', 1) for item in args.source)
    as_c = {item.split('=', 1)[0]: (item.split('=', 1)[1] if '=' in item else '') for item in args.as_c}
    results = probe(args.owners, overrides, as_c=as_c)
    exact = True
    for owner_id, r in results.items():
        extra = ''.join(f', {d}' for d in r['data_segments'])
        print(f"{owner_id}: {r['status']} ({r['bytes']}/{r['expected']} bytes, {r['fixups']} fixups{extra}, {r['source']})")
        if args.hex or r['status'] != 'EXACT':
            print('  reconstructed:', r['reconstructed'])
            print('  original:     ', r['original'])
        exact &= r['status'] == 'EXACT'
    return 0 if exact else 1


if __name__ == '__main__':
    sys.exit(main())
