"""Audit docs/current/asm-provenance.json against the production plan.

Checks performed:
  1. Plan agreement: the set of modules marked ``production: true`` in
     asm-provenance.json must equal exactly the set of active TASM
     production modules in layout/production-plan.json (module entries
     whose source(s) end with ``.ASM``). Modules not in that active set
     must be marked ``production: false`` in the provenance file.
  2. Member completeness: every member of every production module must
     carry ``classification`` and ``confidence`` fields, and neither may
     be (or be missing and default to) UNKNOWN/PENDING_PROBE for the
     audit to pass in --strict mode; PENDING_PROBE members count as
     unknown in the accounting.
  3. Member coverage: the member ids listed for a production module in
     asm-provenance.json must match the members recorded for that module
     in layout/structural-source-modules.json (for grouped modules) or
     layout/manifest.json (for single-owner modules).

Default mode only reports findings (exit 0). Pass --strict to exit
non-zero when the plan disagrees, or any module/member classification is
UNKNOWN/missing/PENDING_PROBE, or a member is missing from an entry.

Also rewrites the generated summary section of docs/current/asm-provenance.md
from the JSON (the narrative sections below the generated header are kept).
"""
import argparse
import sys
from collections import Counter

from reconstruct import ROOT, read_json, write_json

UNKNOWN_LIKE = {'UNKNOWN', 'PENDING_PROBE', None}


def active_plan_modules(plan):
    active = set()
    for module in plan['modules']:
        sources = module.get('sources') or [module.get('source')]
        sources = [s for s in sources if s]
        if any(s.lower().endswith('.asm') for s in sources):
            active.add(module['id'])
    return active


def structural_members(root):
    document = read_json(root / 'layout/structural-source-modules.json')
    return {module['id']: module['members'] for module in document.get('modules', [])}


def audit(root=ROOT):
    plan = read_json(root / 'layout/production-plan.json')
    provenance = read_json(root / 'docs/current/asm-provenance.json')
    structural = structural_members(root)
    manifest = read_json(root / 'layout/manifest.json')
    manifest_ids = {o['id'] for o in manifest['regions']}

    active_plan = active_plan_modules(plan)
    provenance_active = {m['module'] for m in provenance['modules'] if m.get('production')}
    provenance_inactive = {m['module'] for m in provenance['modules'] if not m.get('production')}

    findings = []

    missing_from_provenance = active_plan - provenance_active
    extra_in_provenance = provenance_active - active_plan
    if missing_from_provenance:
        findings.append(f'Plan disagreement: active plan modules missing production=true: '
                         f'{sorted(missing_from_provenance)}')
    if extra_in_provenance:
        findings.append(f'Plan disagreement: provenance production=true modules not active in plan: '
                         f'{sorted(extra_in_provenance)}')
    wrongly_inactive = provenance_inactive & active_plan
    if wrongly_inactive:
        findings.append(f'Modules marked production=false but active in plan: {sorted(wrongly_inactive)}')

    member_classifications = Counter()
    unknown_modules = 0
    unknown_members = 0
    active_production_bytes = 0
    by_classification = Counter()
    by_classification_bytes = Counter()

    for module in provenance['modules']:
        mid = module['module']
        module_cls = module.get('classification')
        module_unknown = module_cls in UNKNOWN_LIKE
        if module.get('production'):
            active_production_bytes += module.get('bytes', 0)
            by_classification[module_cls] += 1
            by_classification_bytes[module_cls] += module.get('bytes', 0)
            if module_unknown:
                unknown_modules += 1
                findings.append(f'Module {mid} has UNKNOWN/missing classification')

            expected_members = None
            if mid in structural:
                expected_members = structural[mid]
            elif mid in manifest_ids:
                expected_members = [mid]
            actual_members = [mem['id'] for mem in module['members']]
            if expected_members is not None and set(expected_members) != set(actual_members):
                missing = set(expected_members) - set(actual_members)
                extra = set(actual_members) - set(expected_members)
                findings.append(f'Module {mid} member mismatch vs structural/manifest source: '
                                 f'missing={sorted(missing)} extra={sorted(extra)}')

            for member in module['members']:
                cls = member.get('classification')
                conf = member.get('confidence')
                member_classifications[cls] += 1
                if cls is None or conf is None:
                    findings.append(f'Member {member.get("id")} of {mid} missing classification/confidence field')
                    unknown_members += 1
                elif cls in UNKNOWN_LIKE:
                    unknown_members += 1
                    findings.append(f'Member {member.get("id")} of {mid} is {cls}')

    promoted = sorted(provenance_inactive)
    summary = {
        'active_production_asm': {
            'count': len(provenance_active),
            'bytes': active_production_bytes,
            'by_classification': dict(by_classification),
        },
        'promoted_from_asm_history': {
            'count': len(promoted),
            'ids': promoted,
        },
        'unknown': {
            'modules': unknown_modules,
            'members': unknown_members,
        },
        'member_classifications': dict(member_classifications),
    }
    return summary, findings, provenance


def render_md(summary, provenance, findings):
    lines = []
    lines.append('# ASM Provenance Audit')
    lines.append('')
    lines.append('Machine-readable form: `docs/current/asm-provenance.json`. '
                  'Audit tool: `tools/audit_asm_provenance.py` '
                  '(report mode by default; `--strict` for a failing exit code).')
    lines.append('')
    lines.append('<!-- GENERATED: the tables below are regenerated from asm-provenance.json'
                  ' by tools/audit_asm_provenance.py. Do not hand-edit them; edit the JSON'
                  ' and re-run the audit instead. Narrative sections after the marker below'
                  ' are preserved by hand. -->')
    lines.append('')
    lines.append('## Summary')
    lines.append('')
    active = summary['active_production_asm']
    lines.append(f"Active production ASM modules: **{active['count']}**, "
                 f"**{active['bytes']}** bytes.")
    lines.append('')
    lines.append('| Classification | Modules |')
    lines.append('|---|---|')
    for cls, count in sorted(active['by_classification'].items(), key=lambda kv: (-kv[1], kv[0] or '')):
        lines.append(f'| {cls} | {count} |')
    lines.append('')
    promoted = summary['promoted_from_asm_history']
    lines.append(f"Promoted from ASM history (no longer active production ASM): "
                 f"**{promoted['count']}** -- {', '.join(promoted['ids']) or 'none'}.")
    lines.append('')
    unknown = summary['unknown']
    lines.append(f"**UNKNOWN/PENDING_PROBE: {unknown['modules']} modules, "
                 f"{unknown['members']} members.**")
    lines.append('')
    lines.append('| Member classification | Count |')
    lines.append('|---|---|')
    for cls, count in sorted(summary['member_classifications'].items(), key=lambda kv: (-kv[1], kv[0] or '')):
        lines.append(f'| {cls} | {count} |')
    lines.append('')
    lines.append('## Modules')
    lines.append('')
    lines.append('| Module | Source | Members | Bytes | Classification | Confidence | Production |')
    lines.append('|---|---|---|---|---|---|---|')
    for module in provenance['modules']:
        member_ids = ', '.join(m['id'] for m in module['members'])
        lines.append(f"| {module['module']} | {module['source']} | {member_ids} | {module['bytes']} | "
                     f"{module['classification']} | {module['confidence']} | {module.get('production')} |")
    lines.append('')
    lines.append('## Member classifications')
    lines.append('')
    lines.append('| Module | Member | Classification | Confidence |')
    lines.append('|---|---|---|---|')
    for module in provenance['modules']:
        for member in module['members']:
            lines.append(f"| {module['module']} | {member['id']} | "
                         f"{member.get('classification')} | {member.get('confidence')} |")
    lines.append('')
    if findings:
        lines.append('## Open audit findings')
        lines.append('')
        for finding in findings:
            lines.append(f'- {finding}')
        lines.append('')
    return '\n'.join(lines) + '\n'


def run(root=ROOT, strict=False):
    summary, findings, provenance = audit(root)
    provenance['summary'] = summary
    write_json(root / 'docs/current/asm-provenance.json', provenance)

    md_path = root / 'docs/current/asm-provenance.md'
    old = md_path.read_text(encoding='utf-8') if md_path.exists() else ''
    marker = '## Notable evidence highlights'
    tail = ''
    if marker in old:
        tail = old[old.index(marker):]
    new_md = render_md(summary, provenance, findings)
    if tail:
        new_md = new_md + '\n' + tail
    md_path.write_text(new_md, encoding='utf-8')

    print('Summary:', summary)
    pending = sorted(m['id'] for module in provenance['modules'] for m in module['members']
                      if m.get('classification') == 'PENDING_PROBE')
    print(f'PENDING_PROBE members ({len(pending)}):', pending)
    if findings:
        print(f'{len(findings)} finding(s):')
        for finding in findings:
            print(' -', finding)
    else:
        print('No findings.')
    if strict and findings:
        return 1
    return 0


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--strict', action='store_true', help='Exit non-zero on any finding')
    args = parser.parse_args()
    sys.exit(run(strict=args.strict))
