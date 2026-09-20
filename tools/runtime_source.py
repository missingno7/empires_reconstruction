"""Measure runtime source by actual assembler emission, not guessed lengths."""
import re
import tempfile
from pathlib import Path
from reconstruct import ROOT, read_json, read_object, sha
from object_cache import compile_cached
from dos_runner import resolve_runner




class AssemblyFailure(ValueError):
    def __init__(self, message, diagnostics):
        super().__init__(message)
        self.diagnostics=diagnostics


def assembly_diagnostics(work, rows):
    diagnostics=[]
    for path in sorted(work.rglob('*.LOG')) + sorted(work.rglob('host.log')):
        for line in path.read_text(errors='replace').splitlines():
            if not re.search(r'\*\*Error\*\*|Fatal|Error [A-Za-z]|Unable to',line,re.I): continue
            match=re.search(r'\.ASM\((\d+)\)',line,re.I)
            emitted=int(match[1]) if match else None
            source_line=next((r['line'] for r in rows if r.get('instrumented_line')==emitted),None)
            item={'message':line.strip(),'source_line':source_line}
            if item not in diagnostics: diagnostics.append(item)
    return diagnostics[:12]


def emission_opcode(code):
    code=re.sub(r'^\s*[A-Za-z_]\w*:\s*','',code).strip().lower()
    words=code.split()
    if len(words)>1 and words[1] in ('db','dw','dd','dq','dt'): return words[1]
    return words[0] if words else ''


def instrument(text):
    from tasm_rules import MACROS, normalized
    output, rows, macro, macro_lines = [], [], False, []
    for number, line in enumerate(text.splitlines(), 1):
        code = line.split(';', 1)[0].strip()
        words = code.lower().split()
        if len(words) > 1 and words[1] == 'macro':
            macro = True
        if macro:
            output.append(line)
            macro_lines.append(line)
            if words and words[0] == 'endm':
                name=macro_lines[0].split()[0].lower()
                if name not in MACROS or normalized('\n'.join(macro_lines)) != normalized(MACROS[name]):
                    raise ValueError('Unapproved runtime macro; exact-encoding quality cannot be self-declared')
                macro = False
                macro_lines=[]
            continue
        directive = (not words or words[0] in ('public', 'extrn', 'assume', 'end', '.8086')
                     or (len(words) > 1 and words[1] in ('segment', 'ends', 'group', 'label', 'equ', 'proc', 'endp'))
                     or code.endswith(':'))
        if words and words[0] in ('org', 'include', 'incbin', 'align', 'even'):
            raise ValueError(f'Runtime emission directive requires supervisor support at line {number}: {code}')
        if not directive:
            label = f'Q{len(rows):04}'
            output.extend([f'public {label}', f'{label} label byte'])
            opcode=emission_opcode(code)
            category = ('raw_unresolved' if opcode in ('db', 'dw', 'dd', 'dq', 'dt') else
                        'intentional_exact_encoding' if opcode in ('jmp_near', 'call_near') else
                        'symbolic_instruction')
            rows.append({'line': number, 'instrumented_line':len(output)+1, 'label': label, 'text': code, 'category': category})
        if words and words[0] == 'end':
            output.extend(['_TEXT segment byte public \'CODE\'', 'public QEND', 'QEND label byte', '_TEXT ends'])
        output.append(line)
    return '\n'.join(output) + '\n', rows


def assemble_runtime(root=ROOT, source=None, cache=True):
    source = source or root / 'asm/RUNTIME_BLOCK.ASM'
    text = source.read_text()
    measured, rows = instrument(text)
    work = Path(tempfile.mkdtemp(prefix='runtime-check-', dir=root / 'build')).resolve()
    staged = work / 'MEASURE.ASM'
    staged.write_text(measured)
    owner = {'id': 'RUNTIME_MEASURE', 'kind': 'MATCHING_ASM',
             'source': staged.relative_to(root).as_posix(), 'build': {}}
    lock = read_json(root / 'layout/toolchain.json')
    try:
        receipts, session = compile_cached(root, [owner], work / 'compile', root / 'toolchain', resolve_runner(lock), lock, cache)
    except ValueError as error:
        raise AssemblyFailure(str(error), assembly_diagnostics(work, rows)) from error
    obj = read_object((work / 'compile' / receipts[owner['id']]['object']).read_bytes())
    offsets = {p['name']: p['offset'] for p in obj.publics}
    for index, row in enumerate(rows):
        row['start'] = offsets[row['label']]
        row['end'] = offsets[rows[index + 1]['label']] if index + 1 < len(rows) else offsets['QEND']
        if row['end'] <= row['start']:
            raise ValueError(f'Unmeasured or overlapping runtime emission at line {row["line"]}')
        row.pop('label')
    blob = obj.segment_bytes('_TEXT')
    if not rows or rows[0]['start'] != 0 or rows[-1]['end'] != len(blob):
        raise ValueError('Runtime source map does not cover the complete emitted object')
    publics = sorted([p for p in obj.publics if not re.fullmatch(r'Q(?:\d{4}|END)', p['name'])],
                     key=lambda p: (p['offset'], p['name']))
    labels = []
    in_text = False
    for line, code in enumerate(text.splitlines(), 1):
        words = code.split(';', 1)[0].strip().split()
        if len(words)>1 and words[1].lower()=='segment': in_text=words[0].upper()=='_TEXT'
        if len(words)>1 and words[1].lower()=='ends': in_text=False
        if in_text and words and (words[0].endswith(':') or len(words)>1 and words[1].lower()=='label'):
            offset=next((r['start'] for r in rows if r['line']>=line),len(blob))
            labels.append({'name':words[0].rstrip(':'),'offset':offset,'line':line})
    return {'labels':labels, 'source_sha256': sha(source.read_bytes()), 'bytes': len(blob), 'sha256': sha(blob),
            'publics': publics, 'fixups': obj.linker_fixups, 'ranges': rows,
            'object': str(work / 'compile' / receipts[owner['id']]['object'])}, blob


def oracle_bytes(root=ROOT):
    """Frozen C source is compiled only as a reference, never as production input."""
    spec = read_json(root / 'recipes/runtime/oracle.json')
    path = root / 'build/regions/RUNTIME_BLOCK.bin'
    if path.exists() and sha(path.read_bytes()) == spec['sha256']:
        return path.read_bytes()
    work = Path(tempfile.mkdtemp(prefix='runtime-oracle-', dir=root / 'build')).resolve()
    lock = read_json(root / 'layout/toolchain.json')
    owner = {'id': 'RUNTIME_BLOCK', 'source': spec['source'], 'kind': 'MATCHING_C', 'build': {}}
    receipts, _ = compile_cached(root, [owner], work, root / 'toolchain', resolve_runner(lock), lock, True)
    obj = read_object((work / receipts['RUNTIME_BLOCK']['object']).read_bytes())
    data = obj.segment_bytes('_TEXT')[:spec['bytes']]
    if sha(data) != spec['sha256']:
        raise ValueError('Frozen runtime oracle does not match its pinned digest')
    path.parent.mkdir(exist_ok=True)
    path.write_bytes(data)
    return data


def quality(measurement, cfg=None, evidence=None):
    totals = {key: 0 for key in ('symbolic_instruction', 'typed_table_data', 'intentional_exact_encoding', 'raw_unresolved')}
    for row in measurement['ranges']:
        category = row['category']
        # DW/DB is typed only inside a supervisor-evidenced data extent.
        if emission_opcode(row['text']) in ('db','dw','dd','dq','dt') and any(
                d['start'] <= row['start'] < row['end'] <= d['end'] for d in (evidence or {}).get('data_ranges', [])):
            category = 'typed_table_data'
        totals[category] += row['end'] - row['start']
        row['category'] = category
    if sum(totals.values()) != measurement['bytes']:
        raise ValueError('Source-quality byte accounting does not partition the runtime')
    return {'total_bytes': measurement['bytes'], **{k + '_bytes': v for k, v in totals.items()},
            'symbolically_recovered_bytes': totals['symbolic_instruction'] + totals['intentional_exact_encoding'],
            'cfg_covered_bytes': sum(i['size'] for i in (cfg or {}).get('instructions', []))}
