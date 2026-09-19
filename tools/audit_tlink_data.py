"""Audit linked DATA contributions against canonical owners; never place bytes."""
import argparse
from pathlib import Path
import re

from reconstruct import ROOT, read_json, write_json


ROW = re.compile(r'^\s*([0-9A-F]+):([0-9A-F]+)\s+([0-9A-F]+)\s+'
                 r'C=(\S+) S=(\S+) G=(\S+) M=(\S+) ACBP=([0-9A-F]+)$')


def contributions(text):
    result = []
    for line in text.splitlines():
        match = ROW.match(line)
        if match:
            frame, offset, length, cls, segment, group, module, acbp = match.groups()
            result.append({'load_start': int(frame, 16) * 16 + int(offset, 16),
                           'length': int(length, 16), 'class': cls, 'segment': segment,
                           'group': group, 'module': module, 'acbp': int(acbp, 16)})
    return result


def audit(manifest, report, map_text):
    rows = contributions(map_text)
    owners = manifest['regions']
    by_code = {o['build']['code_owner']: o for o in owners
               if o.get('build', {}).get('encoder') == 'omf-segment-v1'}
    staged = {Path(s['object']).stem.upper(): s['owner']
              for s in report['relocatable_scaffold'] if s.get('owner')}
    checked = []
    for row in rows:
        if row['segment'] != '_DATA' or not row['length']:
            continue
        code_owner = staged.get(Path(row['module']).stem.upper())
        owner = by_code.get(code_owner)
        entry = dict(row, code_owner=code_owner)
        if owner:
            entry.update(data_owner=owner['id'], expected_load_start=owner['start'] - 512,
                         expected_length=owner['end'] - owner['start'])
            entry['placement_delta'] = row['load_start'] - entry['expected_load_start']
            entry['length_matches'] = row['length'] == entry['expected_length']
        elif code_owner:
            entry['ownership'] = 'no canonical compiled DATA owner; not proven module storage'
        checked.append(entry)
    missing = report['byte_comparison']['mz']['missing_sites']
    relocation_owners = []
    for owner in owners:
        sites = [site for site in missing if owner['start'] <= site + 512 < owner['end']]
        if sites:
            relocation_owners.append({'owner': owner['id'], 'kind': owner['kind'],
                                      'encoder': owner.get('build', {}).get('encoder'),
                                      'missing_sites': sites})
    compiled = sorted(by_code.values(), key=lambda o: o['start'])
    adjacent = [{'left': a['id'], 'right': b['id'],
                 'left_length': a['end'] - a['start'],
                 'right_load_start': b['start'] - 512,
                 'constraint': 'contiguous canonical DATA; test shared compilation or byte-aligned contribution'}
                for a, b in zip(compiled, compiled[1:]) if a['end'] == b['start']]
    return {'format': 'empires-tlink-data-audit-v1',
            'linked_exe_sha256': report['outputs']['exe_sha256'],
            'data_contributions': checked,
            'bss_contributions': [r for r in rows if r['segment'] == '_BSS' and r['length']],
            'missing_relocation_owners': relocation_owners,
            'adjacent_compiled_data_constraints': adjacent,
            'historical_module_proven': False,
            'scope': 'post-link verification only; fixed ownership does not drive linking'}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report', type=Path, default=ROOT / 'build/tlink-structural-report.json')
    parser.add_argument('--output', type=Path, default=ROOT / 'docs/tlink-data-audit.json')
    args = parser.parse_args()
    report = read_json(args.report)
    map_path = Path(report['byte_comparison']['candidate']).with_suffix('.MAP')
    write_json(args.output, audit(read_json(ROOT / 'layout/manifest.json'), report,
                                 map_path.read_text(errors='replace')))


if __name__ == '__main__':
    main()
