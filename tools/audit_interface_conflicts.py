"""Scope the syntactic interface census to production; retain unresolved findings."""
from collections import Counter
from reconstruct import ROOT, read_json, write_json


def spelled(typ):
    """Compact-model spelling: an unqualified data pointer is a far pointer."""
    return ' '.join(typ.replace(' far *', ' *').split()) if 'near' not in typ else typ


def audit(root=ROOT):
    census = read_json(root / 'docs/current/interface-conflicts.json')
    plan = read_json(root / 'layout/production-plan.json')
    active = {s for m in plan['modules'] if m['tool'] == 'TCC.EXE'
              for s in ([m['source']] if 'source' in m else m.get('sources', []))}
    reviewed = {
        'FUNCTION_ABI:fd3da': ('COMPACT_MODEL_SPELLING', 'char * and char far * have the same pointer representation under -mc. This argument-type difference is spelling, not a near/far ABI difference.'),
    }
    # Interfaces recovered with byte-exact module probes (tools/probe_module.py) and full
    # acceptance. Their former census conflicts now only involve inactive reference sources.
    resolutions = [
        {'id':'FUNCTION_ABI:f5321','resolution':'void f5321(struct E far *ev, int count, int step, struct P far *q): the six stack words are two far pointers and two ints; the locals are an int index, an unsigned long deadline and the previous sprite box. F_5321 and caller F_56C6 compile exactly with the typed prototype.'},
        {'id':'FUNCTION_ABI:fa036','resolution':'long fa036(char far *dest, char far *first, ...) walks the far pointer list to a null sentinel. The variadic prototype reproduces the definition exactly; the F_AA1F call site needs all five arguments typed (record argument as struct c470_record far *) because an untyped trailing argument changes the far address construction by four bytes.'},
        {'id':'FUNCTION_ABI:f8480','resolution':'include/DIALOG.H: 20-byte struct dialog shared by f7e07, f8480 (void) and f86c9 (int). All ten static descriptors carry cx=cy=w=lines=-1; F_7E07 reads offsets 12..19.'},
        {'id':'FUNCTION_ABI:f86c9','resolution':'int f86c9(struct dialog far *): returns the selection consumed by F_56C6, F_AA1F, F_AB66, F_CDDD, F_CE00, F_CE2A and M_CB5C_CD23.'},
        {'id':'FUNCTION_ABI:fa28d','resolution':'include/C470.H merges every partial 27-byte view; int fa28d(struct c470_record far *, int, int). RECORD27.H is retired.'},
        {'id':'FUNCTION_ABI:f652a','resolution':'void f652a(int drive) compiles identically; callers already promote to int.'},
        {'id':'FUNCTION_ABI:f7343','resolution':'void f7343(int value) compiles identically; a char prototype at the F_3986 call site drops the CBW and is one byte short, so callers keep int.'},
        {'id':'GLOBAL_TYPE:gb31','resolution':'Census now records the far pointer value with a separate storage qualifier; no declaration conflict remains.'},
        {'id':'GLOBAL_TYPE:gc0cc','resolution':'extern void interrupt (*gc0cc)(void) in F_695E, F_697D and F_699E; census parses function-pointer declarators.'},
        {'id':'GLOBAL_TYPE:q139d','resolution':'F_56C6 now references the shared g139d dialog descriptor; the manifest binding was renamed at the same DGROUP offset.'},
    ]
    findings = []
    for c in census['conflicts']:
        item = {'id': c['id'], 'kind': c['kind']}
        if c['kind'] == 'RECORD_LAYOUT':
            rows = [r for r in c['layouts'] if r['file'] in active or r['file'].startswith('include/')]
            item.update(category='LOCAL_TAG_REUSE', note='Tags are file scoped; matching tag names across files do not establish a shared type or an ABI conflict.', locations=[{'file':r['file'],'line':r['line']} for r in rows])
        else:
            rows = [d for d in c['declarations'] if d['file'] in active or d['file'].startswith('include/')]
            fields = [k for k in c['fields'] if len({str(d[k]) for d in rows if d[k] is not None}) > 1]
            category = 'INACTIVE_REFERENCE_ONLY' if not fields else 'RETURN_TYPE_REVIEW' if c['kind']=='FUNCTION_ABI' and fields==['return_type'] else 'ACTIVE_TYPE_REVIEW'
            note = 'Syntactic finding; not a confirmed runtime bug.'
            variadic = [d for d in rows if d.get('argument_types') and d['argument_types'][-1] == '...']
            if fields and c['kind']=='FUNCTION_ABI' and variadic and all(
                    d['argument_types'] is None or d['argument_types'][-1] == '...' or
                    (d['argument_count'] >= v['argument_count']-1 and d['argument_types'][:v['argument_count']-1] == v['argument_types'][:-1])
                    for v in variadic for d in rows):
                category, note = 'VARIADIC_PROTOTYPE', 'Definition declares a variadic prototype; callers pass compatible fixed arguments plus trailing typed arguments.'
            if fields and c['kind']=='GLOBAL_TYPE' and fields==['type'] and len({spelled(d['type']) for d in rows}) == 1:
                category, note = 'COMPACT_MODEL_SPELLING', 'Unqualified data pointers are far under -mc; these declarations differ only in spelling out far.'
            if fields and c['id'] in reviewed: category, note = reviewed[c['id']]
            item.update(category=category, note=note, active_fields=fields, active_declarations=rows,
                        excluded_declarations=[d for d in c['declarations'] if d not in rows])
        findings.append(item)
    return {'format':'empires-interface-audit-v1','scope':'Review only; no production declaration edits or conflict suppression.',
            'raw_conflicts':len(census['conflicts']), 'active_c_sources':len(active),
            'categories':dict(sorted(Counter(x['category'] for x in findings).items())),
            'resolutions':resolutions, 'findings':findings, 'unsupported':census['unsupported'],
            'limitations':['Original census is syntactic and scans inactive reference sources.',
                           'Production filtering does not add missing preprocessor, typedef, function pointer, or C compatibility analysis.',
                           'Same stack width does not make incompatible C declarations safe to merge.',
                           'No source-language or runtime correctness proof is claimed.']}

if __name__ == '__main__':
    result = audit()
    write_json(ROOT / 'docs/current/interface-audit.json', result)
    print(result['categories'])
