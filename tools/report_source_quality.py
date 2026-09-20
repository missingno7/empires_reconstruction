"""Measure readable-source recovery independently from byte ownership."""
from collections import defaultdict
from pathlib import Path
import re

from reconstruct import ROOT, read_json, write_json


ASM_DB = re.compile(r'^\s*asm\s+db\b', re.IGNORECASE | re.MULTILINE)
ASM = re.compile(r'^\s*asm\b', re.IGNORECASE | re.MULTILINE)
RAW_ASM_DB = re.compile(r'^\s*db\b', re.IGNORECASE | re.MULTILINE)
EXACT_NEAR_JUMP_MACRO = re.compile(
    r'^\s*JMP_NEAR\s+macro\s+target\s*$\n'
    r'^\s*db\s+0e9h\s*$\n'
    r'^\s*dw\s+target-\$-2\s*$\n'
    r'^\s*endm\s*$', re.IGNORECASE | re.MULTILINE)


def classify_source(path):
    """Return the strongest mechanically verifiable source representation."""
    text = path.read_text(errors='strict')
    if ASM_DB.search(text):
        return 'ASM_DB_CAPSULE'
    if ASM.search(text):
        return 'C_WITH_SYMBOLIC_INLINE_ASM'
    return 'MECHANICAL_C'


def classify_asm_source(path):
    text = path.read_text(errors='strict')
    # A single DB in this narrowly defined macro expresses an intentional
    # symbolic near branch whose exact encoding TASM 1.0 otherwise changes to
    # a short branch.  It is not an instruction-byte capsule: the target and
    # displacement remain assembler-resolved symbols.  All other raw DB uses
    # remain capsules until their instructions are recovered symbolically.
    if EXACT_NEAR_JUMP_MACRO.search(text):
        text = EXACT_NEAR_JUMP_MACRO.sub('', text)
    return 'ASM_DB_CAPSULE' if RAW_ASM_DB.search(text) else 'SYMBOLIC_ASM'


def structural_module_members():
    path = ROOT / 'layout/structural-source-modules.json'
    if not path.exists():
        return {}, []
    document = read_json(path)
    if document.get('format') != 'empires-structural-source-modules-v1':
        raise ValueError('Unknown structural source-module format')
    members, modules = {}, []
    for module in document.get('modules', []):
        source = module['source']
        for member in module['members']:
            if member in members:
                raise ValueError(f'Structural source-module member overlap: {member}')
            members[member] = source
        modules.append({'id': module['id'], 'source': source,
                        'members': module['members'],
                        'bytes': module['end'] - module['start']})
    return members, modules


def report(manifest):
    classes = defaultdict(lambda: {'bytes': 0, 'owners': 0, 'sources': set()})
    capsules = []
    module_members, modules = structural_module_members()
    # The complete first 0x1BC load bytes are the pinned compact-model
    # C0C.OBJ contribution.  The fixed oracle still partitions that prefix
    # into small matching owners, but the normal TLINK build consumes C0C.OBJ
    # directly.  Treat it as the legitimate historical startup input rather
    # than misclassifying mechanically preserved oracle fragments as game code.
    startup_begin, startup_end = 512, 512 + 0x1BC
    for owner in manifest['regions']:
        kind = owner['kind']
        if startup_begin <= owner['start'] and owner['end'] <= startup_end:
            level = 'HISTORICAL_STARTUP_OBJECT'
            source = 'toolchain/C0C.OBJ'
        elif kind == 'KNOWN_TOOLCHAIN_LIBRARY':
            level = 'HISTORICAL_LIBRARY'
            source = owner.get('source', kind)
        elif kind in ('MATCHING_C', 'MATCHING_ASM'):
            source = module_members.get(owner['id'], owner['source'])
            path = ROOT / source
            if source.lower().endswith('.asm'):
                level = classify_asm_source(path)
            else:
                level = classify_source(path)
            if level == 'ASM_DB_CAPSULE':
                capsules.append({'owner': owner['id'], 'source': source,
                                 'bytes': owner['end'] - owner['start']})
        else:
            continue
        entry = classes[level]
        entry['bytes'] += owner['end'] - owner['start']
        entry['owners'] += 1
        entry['sources'].add(source)
    levels = []
    for level in ('ASM_DB_CAPSULE', 'C_WITH_SYMBOLIC_INLINE_ASM',
                  'SYMBOLIC_ASM', 'MECHANICAL_C', 'HISTORICAL_STARTUP_OBJECT',
                  'HISTORICAL_LIBRARY'):
        entry = classes[level]
        levels.append({'level': level, 'bytes': entry['bytes'], 'owners': entry['owners'],
                       'sources': len(entry['sources'])})
    capsules.sort(key=lambda item: (-item['bytes'], item['owner']))
    return {'format': 'empires-source-quality-v1',
            'scope': 'Game-owned matching C/ASM plus legitimate historical library inputs',
            'levels': levels,
            'asm_db_capsules': capsules,
            'asm_db_source_files': len({item['source'] for item in capsules}),
            'structural_source_modules': modules,
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
