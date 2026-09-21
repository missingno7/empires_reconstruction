"""Audit per-module compiler flags against the translation-unit rule.

Turbo C 2.0 takes the assembler path (-B: the whole unit is emitted as .ASM and
assembled by TASM, which relaxes some jumps differently from the native object
writer) in exactly two cases: the unit contains an inline ``asm`` statement, or
-B was given on the command line.  A production module that needs ``-B`` but
contains no ``asm`` statement therefore records a flag the historical source
cannot explain on its own: the function must have lived in a .C file together
with an inline-asm neighbour (see tools/probe_tu.py for testing that as one
unit).  ``-k`` (a frame on every function) has no source-level explanation at
all; a module that needs it is either a hand-written assembler routine with a
hand-written frame or a genuine per-file compiler option.

Usage: python tools/audit_tu_flags.py [--strict] [--json PATH]
"""
import argparse
import json
import re
import sys

from reconstruct import ROOT, read_json

ASM_STATEMENT = re.compile(r'(?im)^\s*(?:\w+:\s*)?asm\b')


def audit(root=ROOT):
    plan = read_json(root / 'layout/production-plan.json')
    findings = []
    for module in plan['modules']:
        if module['tool'] != 'TCC.EXE':
            continue
        sources = module.get('sources') or [module['source']]
        has_asm = any(ASM_STATEMENT.search((root / s).read_text(encoding='latin-1')) for s in sources)
        extra = module['build'].get('flags_append', '').split()
        for flag in extra:
            if flag == '-B' and has_asm:
                findings.append({'module': module['id'], 'flag': flag, 'kind': 'REDUNDANT_FLAG',
                                 'detail': 'inline asm already takes the assembler path'})
            elif flag == '-B':
                findings.append({'module': module['id'], 'flag': flag, 'kind': 'UNEXPLAINED_FLAG',
                                 'detail': 'no inline asm in the unit: historically shared a .C file with one'})
            elif flag == '-k':
                findings.append({'module': module['id'], 'flag': flag, 'kind': 'UNEXPLAINED_FLAG',
                                 'detail': 'forced frame: hand-written frame or per-file option'})
            else:
                findings.append({'module': module['id'], 'flag': flag, 'kind': 'OTHER_FLAG', 'detail': ''})
    return {'format': 'empires-tu-flags-audit-v1', 'findings': findings,
            'unexplained': sum(f['kind'] == 'UNEXPLAINED_FLAG' for f in findings)}


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--strict', action='store_true')
    parser.add_argument('--json', help='write the report here')
    args = parser.parse_args(argv)
    report = audit()
    for f in report['findings']:
        print(f"{f['kind']:17} {f['module']:14} {f['flag']:3} {f['detail']}")
    print(f"unexplained flags: {report['unexplained']}")
    if args.json:
        with open(args.json, 'w', encoding='ascii', newline='\n') as handle:
            json.dump(report, handle, indent=2)
            handle.write('\n')
    return 1 if args.strict and report['unexplained'] else 0


if __name__ == '__main__':
    sys.exit(main())
