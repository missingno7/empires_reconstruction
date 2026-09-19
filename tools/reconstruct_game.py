"""Freshly reconstruct all three game files and publish a combined identity/coverage report."""
import argparse
import os
from pathlib import Path
import subprocess
import sys
from collections import Counter

from build_exe import build as build_structural_exe
from reconstruct import ROOT, project_path, read_json, reconstruct, sha, write_json
from reconstruct_archives import rebuild
from pack_archives import pack, verify


def exe_metrics(root, manifest, report):
    original = project_path(root, manifest['original']['path']).read_bytes()
    inventory = read_json(root / 'docs/upstream-inventory.json')
    # Classify only original on-disk ranges, never runtime-generated arena code.
    code_ranges = []
    for entry in inventory['entries']:
        if entry['kind'] != 'UNRECOVERED_MACHINE':
            continue
        extent = entry['extent']
        start, end = extent['file_offset'], extent['file_offset'] + extent['length']
        if start < 0 or end > len(original) or sha(original[start:end]) != extent['sha256']:
            raise ValueError(f"{entry['id']}: classification extent identity differs")
        code_ranges.append((start, end, entry['id']))
    classified = []
    for owner in manifest['regions']:
        if owner['kind'] != 'RAW':
            continue
        for start, end, entry_id in code_ranges:
            lo, hi = max(start, owner['start']), min(end, owner['end'])
            if hi > lo:
                classified.append({'start': lo, 'end': hi, 'owner': owner['id'], 'evidence_id': entry_id})
    ordered = sorted(classified, key=lambda r: r['start'])
    # Upstream includes overlapping entry/parent extents. Count their union;
    # retaining each evidence range below must not inflate byte coverage.
    merged = []
    for row in ordered:
        if merged and row['start'] <= merged[-1][1]:
            merged[-1][1] = max(merged[-1][1], row['end'])
        else:
            merged.append([row['start'], row['end']])
    unresolved = sum(end - start for start, end in merged)
    counts = report['bytes_by_kind']
    linkage = read_json(root / 'docs/linkage-blockers.json')
    return {'total_bytes': report['total_bytes'], 'matching_c_bytes': counts.get('MATCHING_C', 0),
            'matching_asm_bytes': counts.get('MATCHING_ASM', 0),
            'source_derived_code_bytes': counts.get('MATCHING_C', 0) + counts.get('MATCHING_ASM', 0),
            'known_library_runtime_bytes': counts.get('KNOWN_TOOLCHAIN_LIBRARY', 0),
            'known_library_code_bytes': sum(r['end'] - r['start'] for r in manifest['regions'] if r['kind'] == 'KNOWN_TOOLCHAIN_LIBRARY' and r['build']['segment'] == '_TEXT'),
            'known_library_data_bytes': sum(r['end'] - r['start'] for r in manifest['regions'] if r['kind'] == 'KNOWN_TOOLCHAIN_LIBRARY' and r['build']['segment'] == '_DATA'),
            'structured_header_bytes': counts.get('MZ_HEADER', 0),
            'classified_static_data_bytes': sum(r['end'] - r['start'] for r in manifest['regions'] if r['kind'] == 'EXACT_DATA' and r.get('classification') == 'static_data'),
            'classified_embedded_asset_bytes': sum(r['end'] - r['start'] for r in manifest['regions'] if r['kind'] == 'EXACT_DATA' and r.get('classification') == 'embedded_palette'),
            'unresolved_machine_code_bytes': unresolved, 'raw_unknown_bytes': counts.get('RAW', 0) - unresolved,
            'source_proof_units': sum(r['kind'] in ('MATCHING_C', 'MATCHING_ASM') for r in manifest['regions']),
            'historical_source_modules_proven': 0,
            'declared_owner_symbol_bindings': sum(len(r.get('build', {}).get('bindings', {})) for r in manifest['regions']),
            'linker_resolved_bindings': 0,
            'owned_component_bindings': sum('owner' in b for r in manifest['regions'] for b in r.get('build', {}).get('bindings', {}).values()),
            'owned_module_segment_bindings': sum('owner' in b for r in manifest['regions'] for b in r.get('build', {}).get('module_segments', {}).values()),
            'unresolved_external_symbols_in_held_snapshot': linkage['unique_unresolved_symbols'],
            'unresolved_fixup_sites_in_held_snapshot': linkage['unresolved_fixup_sites'],
            'classification_scope': 'Pinned upstream on-disk machine extents intersected with current RAW owners; all other raw bytes remain unknown.',
            'classification_evidence_sha256': sha((root / 'docs/upstream-inventory.json').read_bytes()),
            'held_evidence_sha256': sha((root / 'docs/linkage-blockers.json').read_bytes()),
            'unresolved_machine_ranges': ordered}


def structural_exe_receipt(root, build):
    """Adapt the structural build receipt to the metrics-only EXE interface."""
    manifest = read_json(root / 'layout/manifest.json')
    counts = Counter()
    for owner in manifest['regions']:
        counts[owner['kind']] += owner['end'] - owner['start']
    return {'reconstructed_sha256': build['sha256'], 'total_bytes': build['size'],
            'bytes_by_kind': dict(counts)}


def reconstruct_game(root, output, toolchain, dosbox, exe_mode='structural'):
    output.mkdir(parents=True, exist_ok=True)
    # Individual builds also invalidate this report when they are invoked directly.
    (output / 'game-report.json').unlink(missing_ok=True)
    if exe_mode == 'structural':
        if output != root / 'build':
            raise ValueError('the structural EXE build currently publishes only to the canonical build directory')
        structural_build = build_structural_exe(root, verify=True, dosbox=dosbox)
        exe = structural_exe_receipt(root, structural_build)
    elif exe_mode == 'fixed-oracle':
        structural_build = None
        exe = reconstruct(root, root / 'layout/manifest.json', output, toolchain, dosbox)
    else:
        raise ValueError(f'Unknown EXE build mode {exe_mode!r}')
    archives = rebuild(root, output)
    packed = pack(root, output / 'packed')
    packing_verification = verify(root, output / 'packed', output)
    metrics = exe_metrics(root, read_json(root / 'layout/manifest.json'), exe)
    report = {'status': 'EQUAL', 'files': {'AEPROG.EXE': exe['reconstructed_sha256'],
                                        **{name + '.DAT': data['reconstructed_sha256'] for name, data in archives.items()}},
              'total_bytes': exe['total_bytes'] + sum(r['total_bytes'] for r in archives.values()),
              'exe': metrics, 'archives': archives,
              'build_reconstruction': {'whole_build_reconstruction_complete': False,
                                       'exe_layout': ('structural_tlink2' if exe_mode == 'structural'
                                                      else 'fixed_placement_oracle'),
                                       'structural_exe_build': structural_build,
                                       'dat_layout': 'derived_from_component_order_and_sizes',
                                       'dat_opaque_fallback_resources': sum(r['opaque_fallback_resources'] for r in packed['archives'].values()),
                                       'dat_verification': packing_verification}}
    write_json(output / 'game-report.json', report)
    print(f"Complete game: EQUAL; {report['total_bytes']:,} bytes in three files")
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, default=ROOT / 'build')
    parser.add_argument('--toolchain', type=Path, default=ROOT / 'toolchain')
    parser.add_argument('--dosbox', type=Path, help='force DOSBox for the fixed-placement oracle path')
    parser.add_argument('--fixed-exe-oracle', action='store_true',
                        help='use the older fixed-placement EXE builder for diagnostics')
    args = parser.parse_args()
    if not args.output.resolve().is_relative_to((ROOT / 'build').resolve()):
        parser.error('--output must be within the project build directory')
    try:
        reconstruct_game(ROOT, args.output.resolve(), args.toolchain.resolve(), args.dosbox.resolve() if args.dosbox else None,
                         'fixed-oracle' if args.fixed_exe_oracle else 'structural')
    except (ValueError, OSError, KeyError, subprocess.SubprocessError) as error:
        print(f'FAIL: {error}', file=sys.stderr)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
