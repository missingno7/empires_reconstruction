"""Measure readable-source recovery independently from byte ownership.

Runtime-block accounting rule (v2): asm/RUNTIME_BLOCK.ASM (owner
RUNTIME_BLOCK) is no longer collapsed into a single ASM_DB_CAPSULE just
because it contains `db` lines. Its owner bytes are instead taken verbatim
from the assembler-measured categories in docs/current/runtime-progress.json
(produced by tools/runtime_source.quality, mirrored into
docs/current/status.json): symbolic_instruction_bytes, typed_table_data_bytes,
intentional_exact_encoding_bytes and raw_unresolved_bytes. Those four numbers
are not re-derived here by regex; they are read from that producer and must
sum to exactly the RUNTIME_BLOCK owner's byte span (asserted below). They map
to the levels RUNTIME_SYMBOLIC_ASM, RUNTIME_TYPED_TABLE_DATA,
RUNTIME_INTENTIONAL_EXACT_ENCODING and RUNTIME_RAW_UNRESOLVED respectively.

For every other ASM source, `db` usage is classified conservatively:
ASM_DB_CAPSULE is reserved for `db` bytes that encode raw, unrecovered
instruction opcodes. `db` bytes that are plainly data -- a label declared
under an explicit data block (`... label:` followed by `db`/`dw` table rows,
or `db 'text'` literal/table entries) -- are typed data, not an instruction
capsule. Because general-purpose syntactic parsing cannot always separate
"data db" from "instruction-encoding db" with certainty, the discrimination
here is intentionally conservative and allow-list driven: a source file is
only ever treated as containing typed (non-capsule) `db` data when every
raw `db`/`dw` line in it falls inside a data region named in
ASM_TYPED_DATA_ALLOWLIST below (labels documented, by inspection, as pure
data tables in the module's own recipe/evidence). Anything else with a raw
`db` outside of the one recognized exact-encoding macro remains a capsule.
This keeps the rule mechanically checkable (an explicit, reviewable
allow-list) rather than trying to infer instruction-vs-data intent from
regex alone.
"""
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

# Sources (other than RUNTIME_BLOCK.ASM, which is handled separately from the
# runtime-progress.json measurement) whose entire raw `db`/`dw` content is
# documented, by inspection of the module's own recipe/evidence, as data
# tables rather than instruction-byte encodings. Empty until a module is
# reviewed and added here explicitly; see the module docstring above.
ASM_TYPED_DATA_ALLOWLIST = set()


BANNER = re.compile(r'/\* ---- ([A-Z0-9_]+) \(original code at 0x[0-9A-Fa-f]+\) ---- \*/')


def member_section(text, owner_id):
    """The owner's own section of a merged translation unit, or the whole text."""
    match = re.search(r'/\* ---- ' + re.escape(owner_id) + r' \(original code at', text)
    if match is None:
        return text
    following = BANNER.search(text, match.end())
    return text[match.start():following.start()] if following else text[match.start():]


def classify_source(path, owner_id=None):
    """Return the strongest mechanically verifiable source representation.

    A merged unit is judged per member section: an inline-asm fragment in one
    function does not make the unit's other functions inline-asm C."""
    text = path.read_text(errors='strict')
    if owner_id:
        text = member_section(text, owner_id)
    # Comments that mention asm/ paths or the word asm are not statements.
    text = re.sub(r'/\*.*?\*/', '', text, flags=re.S)
    if ASM_DB.search(text):
        return 'ASM_DB_CAPSULE'
    if ASM.search(text):
        return 'C_WITH_SYMBOLIC_INLINE_ASM'
    return 'MECHANICAL_C'


def classify_asm_source(path, source=None):
    text = path.read_text(errors='strict')
    # A single DB in this narrowly defined macro expresses an intentional
    # symbolic near branch whose exact encoding TASM 1.0 otherwise changes to
    # a short branch.  It is not an instruction-byte capsule: the target and
    # displacement remain assembler-resolved symbols.  All other raw DB uses
    # remain capsules until their instructions are recovered symbolically.
    if EXACT_NEAR_JUMP_MACRO.search(text):
        text = EXACT_NEAR_JUMP_MACRO.sub('', text)
    if not RAW_ASM_DB.search(text):
        return 'SYMBOLIC_ASM'
    if source in ASM_TYPED_DATA_ALLOWLIST:
        return 'SYMBOLIC_ASM'
    return 'ASM_DB_CAPSULE'


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


RUNTIME_LEVEL_MAP = {
    'symbolic_instruction_bytes': 'RUNTIME_SYMBOLIC_ASM',
    'typed_table_data_bytes': 'RUNTIME_TYPED_TABLE_DATA',
    'intentional_exact_encoding_bytes': 'RUNTIME_INTENTIONAL_EXACT_ENCODING',
    'raw_unresolved_bytes': 'RUNTIME_RAW_UNRESOLVED',
}


def report(manifest, runtime_metrics=None):
    classes = defaultdict(lambda: {'bytes': 0, 'owners': 0, 'sources': set()})
    capsules = []
    module_members, modules = structural_module_members()
    if runtime_metrics is None:
        progress_path = ROOT / 'docs/current/runtime-progress.json'
        runtime_metrics = read_json(progress_path)['metrics'] if progress_path.exists() else None
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
            if owner['id'] == 'RUNTIME_BLOCK' or source == 'asm/RUNTIME_BLOCK.ASM':
                if runtime_metrics is None:
                    raise ValueError('RUNTIME_BLOCK owner requires docs/current/runtime-progress.json metrics')
                owner_bytes = owner['end'] - owner['start']
                measured = sum(runtime_metrics[key] for key in RUNTIME_LEVEL_MAP)
                if measured != owner_bytes:
                    raise ValueError(
                        f'Runtime metrics ({measured}) do not sum to RUNTIME_BLOCK owner bytes ({owner_bytes})')
                for key, level in RUNTIME_LEVEL_MAP.items():
                    n = runtime_metrics[key]
                    if n == 0:
                        continue
                    entry = classes[level]
                    entry['bytes'] += n
                    entry['owners'] += 1
                    entry['sources'].add(source)
                continue
            path = ROOT / source
            if source.lower().endswith('.asm'):
                level = classify_asm_source(path, source)
            else:
                level = classify_source(path, owner['id'])
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
                  'HISTORICAL_LIBRARY', 'RUNTIME_SYMBOLIC_ASM',
                  'RUNTIME_TYPED_TABLE_DATA', 'RUNTIME_INTENTIONAL_EXACT_ENCODING',
                  'RUNTIME_RAW_UNRESOLVED'):
        entry = classes[level]
        levels.append({'level': level, 'bytes': entry['bytes'], 'owners': entry['owners'],
                       'sources': len(entry['sources'])})
    capsules.sort(key=lambda item: (-item['bytes'], item['owner']))
    return {'format': 'empires-source-quality-v2',
            'scope': 'Game-owned matching C/ASM plus legitimate historical library inputs',
            'levels': levels,
            'asm_db_capsules': capsules,
            'asm_db_source_files': len({item['source'] for item in capsules}),
            'structural_source_modules': modules,
            'limitations': ('Non-runtime classification is syntactic and allow-list driven for typed '
                            'ASM data (see module docstring); MECHANICAL_C does not claim semantic '
                            'recovery. RUNTIME_BLOCK.ASM bytes are taken from the assembler-measured '
                            'runtime-progress.json categories, not re-derived here. Historical module '
                            'ownership is tracked separately.')}


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
