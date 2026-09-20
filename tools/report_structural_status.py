"""Derive structural checkpoint metrics from canonical ownership and a link report."""
import argparse
from collections import Counter
from pathlib import Path
from reconstruct import ROOT, read_json, write_json


def status(root, report, exe_build=None):
    manifest = read_json(root / 'layout/manifest.json')
    counts, sizes = Counter(), Counter()
    for owner in manifest['regions']:
        counts[owner['kind']] += 1
        sizes[owner['kind']] += owner['end'] - owner['start']
    raw = sorted(({'owner': o['id'], 'bytes': o['end'] - o['start']}
                  for o in manifest['regions'] if o['kind'] == 'RAW'),
                 key=lambda item: -item['bytes'])
    scaffolds = [s for s in report['relocatable_scaffold'] if s['kind'] != 'owner']
    transformed = [s for s in report['relocatable_scaffold'] if s.get('transforms')]
    dgroup_metadata = [s for s in transformed
                        if s['transforms'] == ['Turbo C-compatible empty DGROUP metadata']]
    symbol_transforms = [s for s in transformed if s not in dgroup_metadata]
    source_path = root / 'build/source-data-link-report.json'
    source = read_json(source_path) if source_path.exists() else None
    exact_path = root / 'build/exact-structural-link-report.json'
    exact = read_json(exact_path) if exact_path.exists() else None
    source_quality_path = root / 'docs/source-quality.json'
    source_quality = read_json(source_quality_path) if source_quality_path.exists() else None
    quality_levels = {} if source_quality is None else {
        entry['level']: {'bytes': entry['bytes'], 'owners': entry['owners']}
        for entry in source_quality['levels']
    }
    matching_owners = [owner for owner in manifest['regions']
                       if owner['kind'] in ('MATCHING_C', 'MATCHING_ASM')]
    closure = None
    if exe_build is not None:
        verification = exe_build['verification']
        structural_modules = exe_build.get('structural_source_modules', [])
        collapsed_proof_units = sum(
            max(0, sum(member in {owner['id'] for owner in matching_owners}
                       for member in module['members']) - 1)
            for module in structural_modules)
        closure = {
            'raw_exe_fallback_bytes': sizes['RAW'],
            'source_quality': quality_levels,
            'total_bss_bytes': exe_build['bss']['bytes'],
            'partitioned_bss_bytes': exe_build['bss']['partitioned_source_bytes'],
            'typed_bss_source_bytes': exe_build['bss'].get('typed_source_bytes', 0),
            'aggregate_bss_remainder_bytes': exe_build['bss']['aggregate_remainder_bytes'],
            'isolated_function_proof_units': len(matching_owners) - collapsed_proof_units,
            'reconstructed_shared_modules': len(exe_build['shared_module_stages']) + len(structural_modules),
            'structural_source_modules': structural_modules,
            'active_structural_adapters': exe_build['remaining_structural_adapters'],
            'active_omf_transforms': [stage['fixupp_order_adapter'] for stage in exe_build['shared_module_stages']
                                      if stage['fixupp_order_adapter'] is not None],
            'fixture_dependency': exe_build['fixture_dependency'],
            'unresolved_symbols': exe_build['unresolved_symbols'],
            'relocation_count': exe_build['relocations'],
            'exact_exe_verification': verification,
        }
    # The fixed-placement scaffold remains a useful diagnostic oracle, but it
    # must not be reported as the current executable build once a fresh exact
    # linked receipt exists.  Prefer source-DATA's full-link observations for
    # segment detail and the canonical build receipt for final verification.
    baseline_diagnostic = {
        'linker_sha256': report['linker']['sha256'],
        'link_status': report['status'],
        'link_errors': report['link'].get('errors', []),
        'segments': report['segments'],
        'unresolved_count': report['link']['unresolved_count'],
        'first_code_placement_divergence': report['code_comparison']['first_divergence'],
        'demand_object_present': any(s['kind'] == 'historical_library_demand' for s in scaffolds),
        'synthetic_data_bytes': sum(s.get('data_bytes', 0) for s in scaffolds),
        'synthetic_bss_bytes': sum(s.get('bss_bytes', 0) for s in scaffolds),
        'symbol_transform_owners': len(symbol_transforms),
        'omf_metadata_adapter_owners': len(dgroup_metadata),
        'byte_comparison': {k: v for k, v in report['byte_comparison'].items()
                            if k not in ('candidate', 'oracle')},
    }
    if exe_build is None:
        current = baseline_diagnostic
    else:
        verification = exe_build['verification']
        current = {
            'linker_sha256': exe_build['linker']['sha256'],
            'link_status': exe_build['status'],
            'link_errors': [],
            'segments': source['segments'] if source is not None else report['segments'],
            'unresolved_count': exe_build['unresolved_symbols'],
            'first_code_placement_divergence': None,
            'demand_object_present': False,
            'synthetic_data_bytes': 0,
            'synthetic_bss_bytes': 0,
            'symbol_transform_owners': 0,
            'omf_metadata_adapter_owners': 0,
            'byte_comparison': {
                'available': verification['performed'],
                'candidate_sha256': verification['sha256'],
                'oracle_sha256': verification['expected_sha256'],
                'mz': {'candidate_fields': {'e_crlc': exe_build['relocations']},
                       'relocation_order_equal': verification['relocation_order_equal']},
                'full_file': {'equal': verification['byte_identical']},
            },
        }
    return {
        'format': 'empires-structural-status-v1',
        'ownership': {kind: {'bytes': sizes[kind], 'owners': counts[kind]} for kind in sorted(counts)},
        'largest_raw_owners': raw[:2],
        **current,
        'baseline_diagnostic': baseline_diagnostic if exe_build is not None else None,
        'historical_translation_units': 'open; C0C invariant bytes and module extent strongly evidenced',
        'whole_build_reconstruction_complete': False,
        'exe_closure': closure,
        'source_data_experiment': None if source is None else {
            'status': source['status'],
            'initialized_data_equal': source['byte_comparison']['initialized_data']['equal'],
            'load_image_equal': source['byte_comparison']['load_image']['equal'],
            'text_differing_bytes': source['byte_comparison']['text']['differing_byte_count'],
            'relocation_count': source['byte_comparison']['mz']['candidate_fields']['e_crlc'],
            'missing_relocations': len(source['byte_comparison']['mz']['missing_sites']),
            'extra_relocations': len(source['byte_comparison']['mz']['extra_sites']),
            'oracle_copied_initialized_data_bytes': source['oracle_copied_initialized_data_bytes'],
            'local_raw_source_bytes': source['local_raw_source_bytes'],
            'synthetic_bss_bytes': source['synthetic_bss_bytes'],
            'partitioned_bss_source_bytes': source['partitioned_bss_source_bytes'],
            'unpartitioned_bss_source_bytes': source['unpartitioned_bss_source_bytes'],
        },
        'exact_structural_experiment': None if exact is None else {
            'status': exact['status'],
            'whole_exe_equal': exact['byte_comparison']['full_file']['equal'],
            'sha256': exact['byte_comparison']['candidate_sha256'],
            'relocation_order_equal': exact['byte_comparison']['mz']['relocation_order_equal'],
            'load_image_equal': exact['byte_comparison']['load_image']['equal'],
            'remaining_adapters': exact['remaining_adapters'],
            'whole_build_reconstruction_complete': exact['whole_build_reconstruction_complete'],
        },
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report', type=Path, default=ROOT / 'build/tlink-structural-report.json')
    parser.add_argument('--output', type=Path, default=ROOT / 'docs/structural-status.json')
    args = parser.parse_args()
    build_path = ROOT / 'build/exe-build-report.json'
    build = read_json(build_path) if build_path.exists() else None
    write_json(args.output, status(ROOT, read_json(args.report), build))


if __name__ == '__main__':
    main()
