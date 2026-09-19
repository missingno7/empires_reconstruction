"""Derive structural checkpoint metrics from canonical ownership and a link report."""
import argparse
from collections import Counter
from pathlib import Path
from reconstruct import ROOT, read_json, write_json


def status(root, report):
    manifest = read_json(root / 'layout/manifest.json')
    counts, sizes = Counter(), Counter()
    for owner in manifest['regions']:
        counts[owner['kind']] += 1
        sizes[owner['kind']] += owner['end'] - owner['start']
    raw = sorted(({'owner': o['id'], 'bytes': o['end'] - o['start']}
                  for o in manifest['regions'] if o['kind'] == 'RAW'),
                 key=lambda item: -item['bytes'])
    scaffolds = [s for s in report['relocatable_scaffold'] if s['kind'] != 'owner']
    source_path = root / 'build/source-data-link-report.json'
    source = read_json(source_path) if source_path.exists() else None
    return {
        'format': 'empires-structural-status-v1',
        'ownership': {kind: {'bytes': sizes[kind], 'owners': counts[kind]} for kind in sorted(counts)},
        'largest_raw_owners': raw[:2],
        'linker_sha256': report['linker']['sha256'],
        'link_status': report['status'],
        'link_errors': report['link'].get('errors', []),
        'segments': report['segments'],
        'unresolved_count': report['link']['unresolved_count'],
        'first_code_placement_divergence': report['code_comparison']['first_divergence'],
        'demand_object_present': any(s['kind'] == 'historical_library_demand' for s in scaffolds),
        'synthetic_data_bytes': sum(s.get('data_bytes', 0) for s in scaffolds),
        'synthetic_bss_bytes': sum(s.get('bss_bytes', 0) for s in scaffolds),
        'symbol_transform_owners': sum(bool(s.get('transforms')) for s in report['relocatable_scaffold']),
        'byte_comparison': {k: v for k, v in report['byte_comparison'].items()
                            if k not in ('candidate', 'oracle')},
        'historical_translation_units': 'open; C0C invariant bytes and module extent strongly evidenced',
        'whole_build_reconstruction_complete': False,
        'source_data_experiment': None if source is None else {
            'status': source['status'],
            'initialized_data_equal': source['byte_comparison']['initialized_data']['equal'],
            'text_differing_bytes': source['byte_comparison']['text']['differing_byte_count'],
            'relocation_count': source['byte_comparison']['mz']['candidate_fields']['e_crlc'],
            'missing_relocations': len(source['byte_comparison']['mz']['missing_sites']),
            'extra_relocations': len(source['byte_comparison']['mz']['extra_sites']),
            'oracle_copied_initialized_data_bytes': source['oracle_copied_initialized_data_bytes'],
            'local_raw_source_bytes': source['local_raw_source_bytes'],
            'synthetic_bss_bytes': source['synthetic_bss_bytes'],
        },
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report', type=Path, default=ROOT / 'build/tlink-structural-report.json')
    parser.add_argument('--output', type=Path, default=ROOT / 'docs/structural-status.json')
    args = parser.parse_args()
    write_json(args.output, status(ROOT, read_json(args.report)))


if __name__ == '__main__':
    main()
