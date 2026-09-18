"""Snapshot held-candidate linkage evidence without modifying the upstream project."""
import argparse
from collections import Counter
from pathlib import Path

from reconstruct import ROOT, read_json, sha, write_json


def capture(upstream, root=ROOT):
    source = upstream / 'controls/correspondence/held-by-provider.json'
    evidence = read_json(source)
    owners = [r for r in read_json(root / 'layout/manifest.json')['regions'] if r['kind'] != 'RAW']
    owned = {r['id'] for r in owners}
    held, aliases = [], []
    for row in evidence['held']:
        if row['id'] in owned:
            continue
        entry_path = source.parent / (row['id'] + '.json')
        extent = read_json(entry_path)['extent']
        replacement = next((r for r in owners
                            if r['kind'] in ('MATCHING_C', 'MATCHING_ASM', 'KNOWN_TOOLCHAIN_LIBRARY')
                            and r.get('matching_status') == 'EQUAL'
                            and r['start'] == extent['file_offset']
                            and r['end'] == extent['file_offset'] + extent['length']
                            and r['expected_sha256'] == extent['sha256']), None)
        if replacement is None:
            held.append(row)
        else:
            aliases.append({'held_id': row['id'], 'owner': replacement['id'],
                            'extent_bytes': extent['length'], 'extent_sha256': extent['sha256'],
                            'upstream_entry_sha256': sha(entry_path.read_bytes())})
    symbols = {}
    for entry in held:
        for target in entry.get('undecided', []):
            row = symbols.setdefault(target['symbol'], {'candidates': {}, 'fixup_sites': 0})
            row['candidates'][entry['id']] = entry['extent_bytes']
            row['fixup_sites'] += 1
    ranking = [{'symbol': symbol, 'candidate_count': len(row['candidates']),
                'candidate_bytes': sum(row['candidates'].values()), 'fixup_sites': row['fixup_sites'],
                'candidate_ids': sorted(row['candidates'])} for symbol, row in symbols.items()]
    ranking.sort(key=lambda r: (-r['candidate_count'], -r['candidate_bytes'], r['symbol']))
    report = {'format': 'empires-held-linkage-evidence-v1', 'upstream': str(upstream),
              'source': str(source.relative_to(upstream)), 'source_sha256': sha(source.read_bytes()),
              'scope': 'Historical provider refusals, excluding currently owned IDs and exactly matching owned extents under other IDs; this is an ownership snapshot, not a fresh compiler proof.',
              'held_candidates': len(held), 'held_extent_bytes': sum(r['extent_bytes'] for r in held),
              'reason_counts': dict(Counter(r['reason'] for r in held)),
              'unique_unresolved_symbols': len(ranking),
              'unresolved_fixup_sites': sum(r['fixup_sites'] for r in ranking),
              'symbols': ranking, 'candidates': held, 'resolved_extent_aliases': aliases}
    write_json(root / 'docs/linkage-blockers.json', report)
    print(f'{len(held)} held candidates; {len(ranking)} unresolved symbols')
    for row in ranking[:10]:
        print(f"{row['symbol']:16s} {row['candidate_count']:3d} candidates; {row['candidate_bytes']:6d} bytes")
    return report


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--upstream', type=Path, default=Path('D:/Games/DOS/dos_recosystem/empires_forged'))
    capture(parser.parse_args().upstream)
