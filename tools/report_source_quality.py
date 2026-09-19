"""Measure readable-source recovery independently from byte ownership."""
from collections import defaultdict
from pathlib import Path
import re

from reconstruct import ROOT, read_json, write_json


ASM_DB = re.compile(r'^\s*asm\s+db\b', re.IGNORECASE | re.MULTILINE)
ASM = re.compile(r'^\s*asm\b', re.IGNORECASE | re.MULTILINE)
RAW_ASM_DB = re.compile(r'^\s*db\b', re.IGNORECASE | re.MULTILINE)


def classify_source(path):
    """Return the strongest mechanically verifiable source representation."""
    text = path.read_text(errors='strict')
    if ASM_DB.search(text):
        return 'ASM_DB_CAPSULE'
    if ASM.search(text):
        return 'C_WITH_SYMBOLIC_INLINE_ASM'
    return 'MECHANICAL_C'


def classify_asm_source(path):
    return 'ASM_DB_CAPSULE' if RAW_ASM_DB.search(path.read_text(errors='strict')) else 'SYMBOLIC_ASM'


def report(manifest):
    classes = defaultdict(lambda: {'bytes': 0, 'owners': 0, 'sources': set()})
    capsules = []
    for owner in manifest['regions']:
        kind = owner['kind']
        if kind == 'KNOWN_TOOLCHAIN_LIBRARY':
            level = 'HISTORICAL_LIBRARY'
        elif kind == 'MATCHING_C':
            path = ROOT / owner['source']
            level = classify_source(path)
            if level == 'ASM_DB_CAPSULE':
                capsules.append({'owner': owner['id'], 'source': owner['source'],
                                 'bytes': owner['end'] - owner['start']})
        elif kind == 'MATCHING_ASM':
            level = classify_asm_source(ROOT / owner['source'])
            if level == 'ASM_DB_CAPSULE':
                capsules.append({'owner': owner['id'], 'source': owner['source'],
                                 'bytes': owner['end'] - owner['start']})
        else:
            continue
        entry = classes[level]
        entry['bytes'] += owner['end'] - owner['start']
        entry['owners'] += 1
        entry['sources'].add(owner.get('source', kind))
    levels = []
    for level in ('ASM_DB_CAPSULE', 'C_WITH_SYMBOLIC_INLINE_ASM',
                  'SYMBOLIC_ASM', 'MECHANICAL_C', 'HISTORICAL_LIBRARY'):
        entry = classes[level]
        levels.append({'level': level, 'bytes': entry['bytes'], 'owners': entry['owners'],
                       'sources': len(entry['sources'])})
    capsules.sort(key=lambda item: (-item['bytes'], item['owner']))
    return {'format': 'empires-source-quality-v1',
            'scope': 'Game-owned matching C plus legitimate historical library inputs',
            'levels': levels,
            'asm_db_capsules': capsules,
            'asm_db_source_files': len({item['source'] for item in capsules}),
            'limitations': ('Classification is syntactic. MECHANICAL_C does not claim semantic '
                            'recovery; historical module ownership is tracked separately.')}


def run():
    result = report(read_json(ROOT / 'layout/manifest.json'))
    write_json(ROOT / 'build/source-quality-report.json', result)
    write_json(ROOT / 'docs/source-quality.json', result)
    asm_db = next(item for item in result['levels'] if item['level'] == 'ASM_DB_CAPSULE')
    print(f"ASM DB: {asm_db['bytes']} bytes, {asm_db['owners']} owners, "
          f"{result['asm_db_source_files']} source files")
    return result


if __name__ == '__main__':
    run()
