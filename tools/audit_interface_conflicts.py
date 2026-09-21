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
        {'id':'FUNCTION_ABI:gfx_copy_rect','outcome':'UNIFIED','resolution':'void gfx_copy_rect(int,int,void far *,int): src/BOARDDRW.C\'s prototype (previously unsigned,unsigned,void far*,int) now matches src/LEVEL.C\'s int,int,void far*,int; int/unsigned only differ in comparison codegen, never in argument-push width, so the call sites are unaffected. C_200F_2A70 probe: EXACT (2771/2771 bytes).'},
        {'id':'FUNCTION_ABI:longjmp','outcome':'UNIFIED','resolution':'void longjmp(void far *,int): src/BOARDSCR.C\'s char far* jmp_buf argument (with an explicit cast at the call site) now matches src/LEVEL.C\'s void far*; both are 4-byte far pointers so the cast becomes redundant, not load-bearing. C_31C4_3986 probe: EXACT (2225/2225 bytes).'},
        {'id':'FUNCTION_ABI:keyboard_irq_handler','outcome':'CODEGEN_ALTERNATE_VIEW','resolution':'src/KEYIRQ.C keeps the untyped `extern void keyboard_irq_handler();` address-only reference used only to build the setvect() far pointer; src/KEYIRQH.C keeps its true definition `void interrupt keyboard_irq_handler(void)`. Byte evidence: retyping the KEYIRQ.C declaration to `extern void interrupt keyboard_irq_handler(void);` grows the address-of/cast sequence feeding setvect() -- C_695E_697D probe: wrong compiled extent length 53, expected 50 (+3 bytes). Reverted; baseline reprobed EXACT (50/50 bytes).'},
        {'id':'FUNCTION_ABI:str_concat_far_list','outcome':'DOCUMENTED','resolution':'long str_concat_far_list(char far *, char far *, ...) at the definition (src/STRCATF.C) is genuinely variadic; the F_AA1F call site in src/SLOTMENU.C needs all five arguments typed (record argument as struct c470_record far *) because an untyped trailing argument changes the far address construction by four bytes. No declaration edit made; nothing further to unify.'},
        {'id':'GLOBAL_TYPE:a74a2','outcome':'CODEGEN_ALTERNATE_VIEW','resolution':'src/BRDPAINT.C keeps `char a74a2[][0xbb]` (its gfx_copy_rect(...,a74a2[c],0) call needs the 2-D row-pointer subscript); src/BRDTERR.C keeps the flat `char a74a2[]` (its `a74a2 + i*0xbb` pointer arithmetic needs a scalar char* that advances by 1 byte per unit, not by one 0xbb-byte row). The extent differs by declared shape only, not signedness. Byte evidence: retyping BRDTERR.C to the 2-D shape -- F_4517 probe: MISMATCH at +0x69 (mov dx,0BBh row-stride multiplier replaced by an unrelated immediate). Reverted; baseline reprobed EXACT (279/279 bytes).'},
        {'id':'GLOBAL_TYPE:b437a','outcome':'UNIFIED','resolution':'unsigned char b437a[]: src/TURNLOOP.C (previously plain char) now matches src/BRDPAINT.C. F_2AE2 and F_3A75 probes: both EXACT.'},
        {'id':'GLOBAL_TYPE:board_records','outcome':'CODEGEN_ALTERNATE_VIEW','resolution':'char far *board_records unified across src/BOARDDRW.C and src/BOARDSCR.C (both previously unsigned char far *; C_200F_2A70 and C_31C4_3986 probes: EXACT). src/BRDPAINT.C keeps unsigned char far *board_records: it reads board_records[0x3e5]/[0x3e6] into ints used as pixel coordinates and needs the zero-extend. Byte evidence: retyping BRDPAINT.C to char far* -- F_2AE2 probe: wrong compiled extent length 1759, expected 1762 (-3 bytes, the sign-extend/zero-extend sequence at the two reads shrinks). Reverted; baseline reprobed EXACT (1762/1762 bytes).'},
        {'id':'GLOBAL_TYPE:g96ee','outcome':'UNIFIED','resolution':'unsigned char g96ee[]: src/LEVEL.C (previously plain char, used only as a gfx_copy_rect pointer argument, no arithmetic) now matches src/BOARDDRW.C. C_AF45_C15E probe: EXACT.'},
        {'id':'GLOBAL_TYPE:g9bfc','outcome':'UNIFIED','resolution':'unsigned char g9bfc[]: src/MENURES.C (previously plain char, used only in memmove()) now matches the unsigned char declarations in src/LEVEL.C. F_1D47 probe: EXACT.'},
        {'id':'GLOBAL_TYPE:gbf66','outcome':'UNIFIED','resolution':'unsigned char gbf66[]: src/MENURES.C (previously plain char) now matches src/LEVEL.C, same edit as g9bfc. F_1D47 probe: EXACT.'},
        {'id':'GLOBAL_TYPE:gbfc4','outcome':'UNIFIED','resolution':'unsigned char far *gbfc4: src/TURNLOOP.C (previously char far *, dereferenced only as `*gbfc4 != 0`) now matches src/BRDPAINT.C. F_3A75 probe: EXACT. (recovery/src/F_D386.C\'s char far * declaration is an inactive reference source -- the TASM module M_D386_D3CF is what actually builds -- and was left untouched.)'},
        {'id':'GLOBAL_TYPE:gbfcc','outcome':'CODEGEN_ALTERNATE_VIEW','resolution':'src/BIOSEQP.C keeps `unsigned char gbfcc` (a bitfield-derived value, no comparison); src/RESOURCE.C keeps `char gbfcc` because `gbfcc > 1` compiles to a signed JG. Byte evidence: retyping RESOURCE.C to unsigned char -- C_6266_68AA probe: MISMATCH at +0xB4, original=7F (JG) reconstructed=77 (JA). Reverted; baseline reprobed EXACT (1641/1641 bytes).'},
        {'id':'GLOBAL_TYPE:gc0ee','outcome':'CODEGEN_ALTERNATE_VIEW','resolution':'src/SPRPOOLD.C\'s `struct H *gc0ee` was retyped to `char far *gc0ee` with a local `(struct H far *)gc0ee` cast at the one field access (F_78C3 probe: EXACT, 91/91 bytes) -- it no longer contributes to the conflict. src/HUD.C keeps three `char *gc0ee` (near, unqualified) declarations alongside four `char far *gc0ee` ones; this is pre-existing, already documented in src/HUD.C (F_7162/F_7202/F_7298 comments): giving the near form a far type there would change the pushes the call compiles to. No further edit made.'},
        {'id':'GLOBAL_TYPE:gc0fe','outcome':'DOCUMENTED','resolution':'include/GC0FE.H already documents this as "a genuine conflict, not unified": F_7BFC (src/MENULIST.C) dereferences gc0fe as its own local `struct {int n; struct R *p;}` shape, which does not agree byte-for-byte with the shared far-pointer/record-catalog view F_7964/F_7D91 use. Kept as-is; nothing to change.'},
        {'id':'GLOBAL_TYPE:record_table_root','outcome':'UNIFIED','resolution':'char far *record_table_root: src/BOARDSCR.C (previously unsigned char far *; f32fa() only does pointer arithmetic on it, no sign-sensitive dereference) now matches src/BOARDDRW.C, src/BRDPAINT.C and src/TURNLOOP.C. C_31C4_3986 probe: EXACT.'},
        {'id':'GLOBAL_TYPE:score_panel_x','outcome':'CODEGEN_ALTERNATE_VIEW','resolution':'score_panel_x is `int` everywhere except src/SCOREPNL.C, which deliberately redeclares it `char *` (far): the single gfx_wipe_rect() argument there reads the 4 bytes at &score_panel_x, and the adjacent int score_panel_y falls in the high word, packing both x2 and y2 into one far-pointer push. Confirmed load-bearing (F_9908 probe: EXACT, 90/90 bytes, unchanged); documented with an explanatory comment in src/SCOREPNL.C instead of unifying.'},
        {'id':'GLOBAL_TYPE:ui_gfx_blob','outcome':'CODEGEN_ALTERNATE_VIEW','resolution':'char far *ui_gfx_blob: src/LEVEL.C\'s one unspelled `char *ui_gfx_blob` (line 152) is now spelled `char far *`, matching its own other four declarations and every other active file (C_AF45_C15E probe: EXACT). src/DIALOG.C keeps two `unsigned ui_gfx_blob` declarations: F_8434/F_8453 push the two DGROUP words (offset, segment) as plain ints through a six-word thunk call, already documented in src/DIALOG.C as spelling the extent literally pushes. No further edit made.'},
        {'id':'GLOBAL_TYPE:voice_level_table','outcome':'CODEGEN_ALTERNATE_VIEW','resolution':'src/OPLVOICE.C and src/VOXCHAN.C keep `unsigned char voice_level_table[]` (plain assignment, no arithmetic); src/OPLREG.C keeps `char voice_level_table[]` because `v = voice_level_table[i] * v` compiles to a signed multiply. Byte evidence: retyping OPLREG.C to unsigned char -- C_E095_E54D probe: wrong compiled extent length 1367, expected 1366 (+1 byte). Reverted; baseline reprobed EXACT (1366/1366 bytes).'},
        {'id':'GLOBAL_TYPE:rect_queue_write_ptr','outcome':'UNIFIED','resolution':'char far *rect_queue_write_ptr spelled out consistently: src/BOARDSCR.C\'s second declarator and src/LEVEL.C:213\'s declarators now say `far` explicitly instead of relying on the -mc default. C_31C4_3986 and C_AF45_C15E probes: both EXACT.'},
        {'id':'GLOBAL_TYPE:resource_stripe_table','outcome':'UNIFIED','resolution':'char far *resource_stripe_table spelled out consistently: src/LEVEL.C:213\'s `resource_ptr_table[]`/`resource_stripe_table` declarators now say `far` explicitly. C_AF45_C15E probe: EXACT (same edit/probe as rect_queue_write_ptr).'},
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
    resolved_ids = {r['id'] for r in resolutions}
    open_findings = [f for f in findings if f['id'] not in resolved_ids]
    resolved_findings = [f for f in findings if f['id'] in resolved_ids]
    return {'format':'empires-interface-audit-v1','scope':'Review only; no production declaration edits or conflict suppression.',
            'raw_conflicts':len(census['conflicts']), 'active_c_sources':len(active),
            'categories':dict(sorted(Counter(x['category'] for x in findings).items())),
            'open_categories':dict(sorted(Counter(x['category'] for x in open_findings).items())),
            'resolutions':resolutions, 'findings':findings,
            'open_findings':open_findings, 'resolved':resolved_findings,
            'unsupported':census['unsupported'],
            'limitations':['Original census is syntactic and scans inactive reference sources.',
                           'Production filtering does not add missing preprocessor, typedef, function pointer, or C compatibility analysis.',
                           'Same stack width does not make incompatible C declarations safe to merge.',
                           'No source-language or runtime correctness proof is claimed.']}

if __name__ == '__main__':
    result = audit()
    write_json(ROOT / 'docs/current/interface-audit.json', result)
    print(result['categories'], 'open:', result['open_categories'])
