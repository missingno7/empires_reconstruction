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
    return {
        'format': 'empires-structural-status-v1',
        'ownership': {kind: {'bytes': sizes[kind], 'owners': counts[kind]} for kind in sorted(counts)},
        'largest_raw_owners': raw[:2],
        'linker_sha256': report['linker']['sha256'],
        'link_status': report['status'],
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
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report', type=Path, default=ROOT / 'build/tlink-structural-report.json')
    parser.add_argument('--output', type=Path, default=ROOT / 'docs/structural-status.json')
    args = parser.parse_args()
    write_json(args.output, status(ROOT, read_json(args.report)))


if __name__ == '__main__':
    main()
