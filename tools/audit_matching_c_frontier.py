"""Audit whether pinned executable-code evidence leaves a promotable C/ASM gap."""
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


def load(path):
    return json.loads((ROOT / path).read_text())


def covered(start, end, regions):
    return any(r['start'] <= start and end <= r['end'] and
               r['kind'] in ('MATCHING_C', 'MATCHING_ASM', 'KNOWN_TOOLCHAIN_LIBRARY')
               for r in regions)


def audit():
    manifest = load('layout/manifest.json')
    inventory = load('docs/upstream-inventory.json')
    held = load('docs/linkage-blockers.json')
    regions = manifest['regions']
    c = [r for r in regions if r['kind'] == 'MATCHING_C']
    asm = [r for r in regions if r['kind'] == 'MATCHING_ASM']
    raw = [r for r in regions if r['kind'] == 'RAW']
    proven = [e for e in inventory['entries']
              if e['kind'] in ('MATCHING_C', 'MATCHING_ASM') and e['verdict'] == 'EQUAL']
    unowned_proven = []
    for entry in proven:
        start = entry['extent']['file_offset']
        end = start + entry['extent']['length']
        if not covered(start, end, regions):
            unowned_proven.append(entry['id'])
    unresolved_in_raw = []
    for entry in inventory['entries']:
        if entry['kind'] != 'UNRECOVERED_MACHINE':
            continue
        start = entry['extent']['file_offset']
        end = start + entry['extent']['length']
        if any(r['start'] < end and start < r['end'] for r in raw):
            unresolved_in_raw.append(entry['id'])
    result = {
        'format': 'empires-matching-c-frontier-audit-v1',
        'status': 'EXTERNAL_CODE_CANDIDATES_EXHAUSTED' if not unowned_proven and not unresolved_in_raw and not asm else 'OPEN',
        'matching_c': {'owners': len(c), 'bytes': sum(r['end'] - r['start'] for r in c)},
        'matching_asm': {'owners': len(asm), 'bytes': sum(r['end'] - r['start'] for r in asm)},
        'raw': {'owners': len(raw), 'bytes': sum(r['end'] - r['start'] for r in raw)},
        'pinned_proven_code_entries': len(proven),
        'unowned_pinned_proven_code_entries': sorted(unowned_proven),
        'unrecovered_machine_entries_intersecting_raw': sorted(unresolved_in_raw),
        'held_candidates': held['held_candidates'],
        'unresolved_symbols': held['unique_unresolved_symbols'],
        'scope': 'Pinned upstream correspondence and the current executable ownership map; this does not claim that a novel decompilation is impossible.',
    }
    return result


if __name__ == '__main__':
    result = audit()
    print(json.dumps(result, indent=2))
