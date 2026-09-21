"""Inventory remaining ASM sources and prepare a fail-closed grinder handover.

Does not start workers or promote C probes. Use --check after runtime refresh.
"""
import argparse
import re
from reconstruct import ROOT, read_json, write_json, sha

C_REVIEW = {
}

def prepare(root=ROOT):
    plan = read_json(root / 'layout/production-plan.json')
    inventory = []
    for module in plan['modules']:
        if module['tool'] != 'TASM.EXE':
            continue
        text = (root / module['source']).read_text()
        evidence = []
        for number, line in enumerate(text.splitlines(), 1):
            instruction = line.split(';', 1)[0].strip()
            if re.search(r'(?i)\b(iret|int|in|out|cli|sti|lodsb|lodsw|movsb|movsw|stosb|stosw|xlat|loop|pushf|popf)\b|\b(push|pop)\s+(ds|es|ax|cx)\b|\borg\b', instruction):
                evidence.append({'line': number, 'instruction': instruction})
        sources = []
        for member in module['members']:
            path = root / 'src' / (member + '.C')
            if path.exists():
                body = path.read_text()
                # Source-form triage only: comments do not count as inline ASM.
                clean = re.sub(r'/\*.*?\*/|//[^\n]*', '', body, flags=re.S)
                sources.append({'path': path.relative_to(root).as_posix(),
                                'sha256': sha(path.read_bytes()),
                                'inline_asm_tokens': len(re.findall(r'\basm\b', clean))})
        ident = module['id']
        if ident == 'RUNTIME_BLOCK':
            route, reason = 'EXISTING_RUNTIME_CARDS', 'Use current bounded runtime cards; do not convert the whole block to C.'
        elif ident == 'M_DDD9_DF98':
            route, reason = 'HELD_SUPERVISOR', 'Six compiler-path probes completed: byte matches, unresolved shared-module relocation order. See c-path-probes.json.'
        elif ident in C_REVIEW:
            route, reason = 'HELD_C_RECONSTRUCTION', C_REVIEW[ident]
        else:
            route, reason = 'HELD_ORIGIN_REVIEW', 'No existing ordinary-C candidate for a bounded flag-only test. Instruction observations do not prove source language.'
        inventory.append({'id': ident, 'source': module['source'], 'source_sha256': sha((root/module['source']).read_bytes()),
                          'range_file_offsets': [module['start'],module['end']], 'bytes':module['end']-module['start'],
                          'route':route, 'reason':reason, 'existing_c_sources':sources, 'instruction_observations':evidence})
    queue = read_json(root / 'docs/current/grinder-queue.json')
    tasks = [t for t in queue['tasks'] if t['difficulty'] == 'CHEAP']
    return {'format':'empires-grinder-readiness-v1', 'input_fingerprint':queue['input_fingerprint'],
            'run_started':False, 'source_inventory':inventory,
            'compiler_path_checks':'docs/current/c-path-probes.json',
            'ready_runtime_cards':len(tasks), 'ready_c_cards':0,
            'ready_card_paths':[t['card'] for t in tasks],
            'held_c_reconstruction':[{'id':k,'scope':v,'status':'HELD_NOT_EXECUTABLE',
                'completion_gate':'Ordinary C candidate, exact whole extent and public/binding evidence; fresh full EXE and ordered relocation acceptance. Never insert DB/padding or patch objects.'} for k,v in C_REVIEW.items()],
            'run_instructions':'docs/current/grinder-run-prompt.md',
            'stop_condition':'Stop when reconstruction_factory.py next returns null; never fall through to held C or supervisor tasks.'}

if __name__ == '__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--check',action='store_true')
    args=parser.parse_args()
    result=prepare()
    path=ROOT/'docs/current/grinder-readiness.json'
    if args.check:
        if not path.exists() or read_json(path)!=result:
            raise SystemExit('STALE: regenerate grinder-readiness after accepted queue refresh')
    else:
        write_json(path,result)
    print(f"{'PASS' if args.check else 'Prepared'}: {len(result['source_inventory'])} ASM modules inventoried; {result['ready_runtime_cards']} runtime CHEAP cards; {result['ready_c_cards']} safe flag-only C cards; no worker started")
