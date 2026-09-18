"""Replace only RAW owners with freshly verified upstream C/ASM/library regions.

Unlike the initial bulk importer, this preserves all existing source owners.
All candidates must pass before any canonical source/layout files are written.
Historical receipts supply bindings; current bytes always require a fresh proof.
"""
import argparse
import copy
import os
from pathlib import Path
import re
import tempfile

from mz import MZ
from reconstruct import (ROOT, bind_region, compile_sources, library_candidate,
                         library_modules, mismatch, project_path, read_json,
                         read_object, sha, validate_layout, write_json)


def owner_from_upstream(upstream, name, original, profile, profile_sha256, modules):
    base = upstream / 'controls/correspondence'
    path = base / (name + '.json')
    record_path = base / 'records' / (name + '.json')
    entry, record = read_json(path), read_json(record_path)
    kind = entry['kind']
    if kind not in ('MATCHING_C', 'MATCHING_ASM', 'KNOWN_TOOLCHAIN_LIBRARY'):
        raise ValueError(f'{name}: no existing source/library proof to reuse')
    if entry['verdict'] != 'EQUAL' or record['verdict'] != 'EQUAL' or record['comparison']['excluded']:
        raise ValueError(f'{name}: incomplete upstream proof')
    if record['extent']['image_sha256'] != sha(original):
        raise ValueError(f'{name}: upstream proof uses a different original')
    extent = entry['extent']
    start, end = extent['file_offset'], extent['file_offset'] + extent['length']
    if MZ.parse(original).file_offset(extent['address']) != start or sha(original[start:end]) != extent['sha256']:
        raise ValueError(f'{name}: upstream extent differs from original')
    declared = {b['symbol']: b for b in profile['layout']['bindings']}
    declared.update({b['symbol']: b for b in entry.get('bindings', [])})
    bindings, segments = {}, {}
    for used in record['comparison']['bindings_used']:
        symbol, evidence = used['symbol'], used['model_key']
        if symbol in ('_TEXT', 'DGROUP'):
            continue
        if symbol in ('_DATA', '_BSS'):
            segments[symbol] = {'offset': used['address'], 'coordinate': 'DGROUP_offset', 'evidence': evidence}
            continue
        if symbol in declared:
            binding = declared[symbol]
            if 'value' in binding:
                raise ValueError(f'{name}: written-value binding needs review: {symbol}')
            frame = binding.get('frame')
            if frame not in (None, '_TEXT', 'DGROUP'):
                raise ValueError(f'{name}: unknown binding frame {frame}')
            value = binding['address'] - profile['layout']['frames'].get(frame, 0)
            coordinate = 'DGROUP_offset' if frame == 'DGROUP' else 'code_offset'
        elif used.get('resolved_by') == 'library public':
            # Established upstream DS offsets become explicit fixed metadata.
            # Never repeat the upstream operand-voting process during a build.
            value, coordinate = used['address'], 'DGROUP_offset'
        else:
            match = re.fullmatch(r'_?[gbwsa]([0-9a-f]{1,6})', symbol)
            function = re.fullmatch(r'_?f([0-9a-f]{1,6})', symbol)
            if not (match or function):
                raise ValueError(f'{name}: undeclared target {symbol}')
            value = int((match or function).group(1), 16)
            coordinate = 'DGROUP_offset' if match else 'code_offset'
        bindings[symbol] = {'offset': value, 'coordinate': coordinate, 'evidence': evidence}
    build = {'segment': entry['source']['segment'], 'bindings': bindings, 'module_segments': segments}
    source_bytes = None
    provenance = {'project': 'empires_forged', 'entry': path.relative_to(upstream).as_posix(),
                  'entry_sha256': sha(path.read_bytes()), 'record_sha256': sha(record_path.read_bytes()),
                  'profile_sha256': profile_sha256}
    if kind == 'KNOWN_TOOLCHAIN_LIBRARY':
        module_name = entry['source']['library_module']
        if module_name not in modules:
            raise ValueError(f'{name}: module not found in pinned CC.LIB')
        module_digest = sha(modules[module_name])
        proof_digest = next(p['digest'] for p in record['verdict_record']['operands'] if p['role'] == 'candidate')
        if proof_digest != 'sha256:' + module_digest:
            raise ValueError(f'{name}: library module differs from upstream proof')
        source = 'toolchain/CC.LIB'
        build.update(library='CC.LIB', library_module=module_name, module_sha256=module_digest)
        provenance['library_sha256'] = next(f['sha256'] for f in profile['toolchain']['files'] if f['path'].endswith('/CC.LIB'))
    else:
        source_path = base / entry['source']['path']
        source_bytes = source_path.read_bytes()
        source = ('src/' if kind == 'MATCHING_C' else 'asm/') + source_path.name
        build.update(public=entry['source']['public'], span=entry['source'].get('span', 1),
                     flags_append=entry.get('build', {}).get('flags_append', ''))
        provenance['source_sha256'] = sha(source_bytes)
        provenance['upstream_record_source_sha256'] = record['source']['sha256']
        normalized = source_bytes.replace(b'\r\n', b'\n').replace(b'\n', b'\r\n')
        provenance['upstream_source_receipt_current'] = record['source']['sha256'] in (sha(source_bytes), sha(normalized))
    return {'id': name, 'start': start, 'end': end, 'kind': kind,
            'classification': 'runtime_library' if kind == 'KNOWN_TOOLCHAIN_LIBRARY' else 'code',
            'source': source, 'original_symbol': name, 'artifact': f'regions/{name}.bin',
            'expected_sha256': extent['sha256'], 'matching_status': 'EQUAL',
            'build': build, 'provenance': provenance}, source_bytes


def replace_raw_owners(manifest, candidates, original):
    """Pure partition operation; every candidate must be wholly within one RAW owner."""
    validate_layout(manifest)
    result = copy.deepcopy(manifest)
    regions = []
    pending = sorted(candidates, key=lambda r: r['start'])
    if len({r['id'] for r in pending}) != len(pending):
        raise ValueError('Duplicate promotion IDs')
    def raw(start, end, previous):
        if start == end:
            return
        owner = copy.deepcopy(previous)
        source = f'raw/{start:06X}-{end:06X}.bin'
        owner.update(id=f'RAW_{start:06X}', start=start, end=end, source=source,
                     artifact=source, expected_sha256=sha(original[start:end]))
        regions.append(owner)
    for existing in result['regions']:
        inside = [r for r in pending if r['start'] < existing['end'] and r['end'] > existing['start']]
        if not inside:
            regions.append(existing)
            continue
        if existing['kind'] != 'RAW':
            raise ValueError(f"Cannot replace non-RAW owner {existing['id']}")
        cursor = existing['start']
        for owner in inside:
            if owner['start'] < cursor or owner['end'] > existing['end']:
                raise ValueError(f"Overlapping or cross-owner promotion: {owner['id']}")
            raw(cursor, owner['start'], existing)
            regions.append(copy.deepcopy(owner))
            cursor = owner['end']
            pending.remove(owner)
        raw(cursor, existing['end'], existing)
    if pending:
        raise ValueError('Promotion outside existing ownership')
    result['regions'] = regions
    validate_layout(result)
    return result


def promote(upstream, ids, all_libraries, dosbox, toolchain):
    manifest = read_json(ROOT / 'layout/manifest.json')
    original = project_path(ROOT, manifest['original']['path']).read_bytes()
    if sha(original) != manifest['original']['sha256'] or len(original) != manifest['original']['size']:
        raise ValueError('Original identity mismatch')
    lock = read_json(ROOT / 'layout/toolchain.json')
    base = upstream / 'controls/correspondence'
    profile_path = base / 'profile.tc20-tasm10-dosbox.json'
    profile = read_json(profile_path)
    selected = set(ids)
    if all_libraries:
        selected.update(read_json(p)['id'] for p in base.glob('LIB_*.json')
                        if read_json(p)['kind'] == 'KNOWN_TOOLCHAIN_LIBRARY')
    existing_ids = {r['id'] for r in manifest['regions']}
    selected -= existing_ids
    if not selected:
        print('No new owners to promote.')
        return
    library_hash = next(p['sha256'] for p in lock['libraries'] if p['path'] == 'CC.LIB')
    modules = library_modules(toolchain / 'CC.LIB', library_hash) if any(n.startswith('LIB_') for n in selected) else {}
    candidates, sources = [], {}
    for name in sorted(selected):
        owner, source_bytes = owner_from_upstream(upstream, name, original, profile, sha(profile_path.read_bytes()), modules)
        candidates.append(owner)
        if source_bytes is not None:
            target = project_path(ROOT, owner['source'])
            if target.exists() and target.read_bytes() != source_bytes:
                raise ValueError(f'Refusing to overwrite existing source {target}')
            sources[owner['source']] = source_bytes
    proposed = replace_raw_owners(manifest, candidates, original)
    (ROOT / 'build').mkdir(exist_ok=True)
    work = Path(tempfile.mkdtemp(prefix='promotion-', dir=ROOT / 'build')).resolve()
    for source, data in sources.items():
        path = project_path(work, source)
        path.parent.mkdir(exist_ok=True)
        path.write_bytes(data)
    compiled = [r for r in candidates if r['kind'] in ('MATCHING_C', 'MATCHING_ASM')]
    receipts, environment = {}, None
    if compiled:
        receipts, environment = compile_sources(work, compiled, work / 'session', toolchain, dosbox, lock)
    proof = {'status': 'EQUAL', 'original_sha256': sha(original), 'session': environment, 'regions': []}
    for owner in candidates:
        if owner['kind'] == 'KNOWN_TOOLCHAIN_LIBRARY':
            module = library_candidate(owner, modules)
        else:
            module = read_object((work / 'session' / receipts[owner['id']]['object']).read_bytes())
        data, detail = bind_region(owner, module, MZ.parse(original), manifest['frames'])
        mismatch(original[owner['start']:owner['end']], data, owner)
        proof['regions'].append({'id': owner['id'], 'sha256': sha(data), 'bindings': detail,
                                 'compile': receipts.get(owner['id'])})
    write_json(work / 'proof.json', proof)
    # Publication only after every candidate and the proposed partition passed.
    for name in ('report.json', 'AEPROG.EXE', 'game-report.json'):
        (ROOT / 'build' / name).unlink(missing_ok=True)
    for source, data in sources.items():
        project_path(ROOT, source).write_bytes(data)
    for owner in proposed['regions']:
        if owner['kind'] == 'RAW':
            path = project_path(ROOT, owner['source'])
            path.parent.mkdir(exist_ok=True)
            path.write_bytes(original[owner['start']:owner['end']])
    pending_manifest = ROOT / 'layout/manifest.json.pending'
    write_json(pending_manifest, proposed)
    pending_manifest.replace(ROOT / 'layout/manifest.json')
    print(f'Promoted {len(candidates)} owners, {sum(r["end"]-r["start"] for r in candidates):,} bytes; fresh proof: {work / "proof.json"}')
    print('Run python tools/reconstruct.py to verify the complete reconstruction.')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--upstream', type=Path, default=Path('D:/Games/DOS/dos_recosystem/empires_forged'))
    parser.add_argument('--ids', nargs='*', default=[])
    parser.add_argument('--libraries', action='store_true', help='Promote all proven, not-yet-owned library extents')
    parser.add_argument('--dosbox', type=Path, default=Path(os.environ.get('DOSBOX', 'C:/Program Files/DOSBox Staging/dosbox.exe')))
    parser.add_argument('--toolchain', type=Path, default=ROOT / 'toolchain')
    args = parser.parse_args()
    promote(args.upstream, args.ids, args.libraries, args.dosbox.resolve(), args.toolchain.resolve())
