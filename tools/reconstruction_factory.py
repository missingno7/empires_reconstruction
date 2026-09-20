"""Deterministic current reports and bounded reconstruction cards."""
import argparse
import base64
from collections import Counter
import hashlib
import json
import re
from pathlib import Path
from reconstruct import ROOT, read_json, write_json, sha
from runtime_source import assemble_runtime, quality, oracle_bytes
from runtime_cfg import runtime_cfg
from interface_census import census


from factory_inputs import input_fingerprint


def exact_measure(root=ROOT):
    measurement, blob=assemble_runtime(root)
    spec=read_json(root/'recipes/runtime/oracle.json')
    if measurement['sha256'] != spec['sha256']:
        raise ValueError('Runtime bytes differ; use check_candidate.py for diagnostics before refreshing')
    if measurement['publics'] != sorted(spec['publics'],key=lambda p:(p['offset'],p['name'])):
        raise ValueError('Runtime publics differ')
    if measurement['fixups'] != spec['fixups']:
        raise ValueError('Runtime fixups differ')
    return measurement,blob


def blocked_ranges(state):
    """Failure ownership survives card regrouping and partial neighboring edits."""
    result=[]
    for ident,record in state.items():
        if record.get('status')!='BLOCKED_SUPERVISOR': continue
        match=re.fullmatch(r'RUNTIME_BLOCK:([0-9A-Fa-f]+)-([0-9A-Fa-f]+)',ident)
        extent=record.get('range') or ([int(x,16) for x in match.groups()] if match else None)
        if not extent or len(extent)!=2 or not 0<=extent[0]<extent[1]<=6571:
            raise ValueError('Invalid blocked runtime extent: '+ident)
        result.append({'id':ident,**record,'range':extent})
    return result


def candidates(measurement,cfg,state):
    ins={i['offset']:i for i in cfg['instructions']}
    occupied={b for i in ins.values() for b in range(i['offset'],i['offset']+i['size'])}
    boundaries=set(ins)|{i['offset']+i['size'] for i in ins.values()}
    raw=[r for r in measurement['ranges'] if r['category']=='raw_unresolved']
    groups=[]
    def row_is_code(row):
        return all(b in occupied for b in range(row['start'],row['end']))
    branch_sites={e['source'] for e in cfg['edges'] if e['kind']!='FALLTHROUGH'}
    region_boundaries={r[k] for r in cfg.get('boundary_regions',[]) for k in ('start','end')}
    failures=blocked_ranges(state)
    region_boundaries.update(p for f in failures for p in f['range'])
    # Keep each unrolled card on complete repeated-body boundaries where raw
    # emission permits it. Existing source rows still define legal edit scope.
    for region in cfg.get('boundary_regions',[]):
        if region['classification']=='UNROLLED_CODE':
            stride=15 if region['start']==0x307 else 21
            region_boundaries.update(range(region['start'],region['end'],(96//stride)*stride))
    for row in raw:
        if (groups and groups[-1][-1]['end']==row['start']
            and row['start'] not in region_boundaries
            and row_is_code(groups[-1][-1])==row_is_code(row)
            and (sum(groups[-1][0]['start']<=b<row['start'] for b in branch_sites)<4 or row['start'] not in boundaries) and
            (row['start']-groups[-1][0]['start']<96 or row['start'] not in boundaries)):
            groups[-1].append(row)
        else: groups.append([row])
    cards=[]
    from runtime_cfg import target_index
    _, owners, frame = target_index()
    globals_ = {}
    owner_by_id = {o['id']:o for o in owners}
    for owner in owners:
        for symbol, binding in owner.get('build',{}).get('bindings',{}).items():
            if binding.get('coordinate') != 'DGROUP_offset': continue
            address = binding.get('offset')
            if address is None and binding.get('owner') in owner_by_id:
                address = owner_by_id[binding['owner']]['start']-512-frame+binding.get('addend',0)
            if address is not None: globals_.setdefault(address, set()).add(symbol)
    for rows in groups:
        start,end=rows[0]['start'],rows[-1]['end']
        code=all(b in occupied for b in range(start,end)) and start in ins and end in boundaries
        indirect=[i for i in cfg['indirect_sites'] if start<=i['offset']<end and not i['complete']]
        issues=[i for i in cfg['issues'] if start<=i['offset']<end]
        branches=[e for e in cfg['edges'] if start<=e['source']<end and e['kind']!='FALLTHROUGH']
        unresolved=[e for e in branches if e['resolution']['classification'] in ('UNKNOWN_ADDRESS','UNKNOWN_INTERIOR_ADDRESS','KNOWN_STRUCTURAL_MODULE')]
        roots=sorted({r for b in cfg['basic_blocks'] if b['start']<end and start<b['end'] for r in b['roots']})
        difficulty='SUPERVISOR' if not code or indirect or issues or unresolved else 'MEDIUM' if len(branches)>4 else 'CHEAP'
        ident=f'RUNTIME_BLOCK:{start:04X}-{end:04X}'
        overlapping=[f for f in failures if f['range'][0]<end and start<f['range'][1]]
        blocked=overlapping[0] if overlapping else None
        if blocked: difficulty='SUPERVISOR'
        reason=('Unknown code/data boundary' if not code else 'Indirect control flow requires evidence' if indirect else
                'Overlapping/invalid CFG' if issues else 'Unresolved branch or call target' if unresolved else blocked.get('reason') if blocked else None)
        priority=(10000 if difficulty=='CHEAP' else 1000 if difficulty=='MEDIUM' else 0)+end-start-10*len(branches)
        neighbors=[{k:r[k] for k in ('start','end','category')} for r in measurement['ranges']
                   if r['end']==start or r['start']==end]
        card={'id':ident,'priority':priority,'kind':'SYMBOLIC_ASM_RECOVERY','difficulty':difficulty,
              'risk':'LOW' if difficulty=='CHEAP' else 'MEDIUM' if difficulty=='MEDIUM' else 'HIGH',
              'range':[start,end],'range_hex':[f'0x{start:04X}',f'0x{end:04X}'],'coordinate':'runtime-owner-relative, half-open',
              'bytes':end-start,'raw_bytes_remaining':end-start,'source':'asm/RUNTIME_BLOCK.ASM',
              'source_lines':[rows[0]['line'],rows[-1]['line']],'known_roots':roots,
              'dependencies':sorted({t['name'] for t in cfg.get('typed_tables',[]) if any(start<=v<end for v in t['targets']+t['consumers'])} | {d for r in cfg.get('boundary_regions',[]) if r['start']<end and start<r['end'] for d in r['dependencies']}),
              'unresolved_targets':unresolved, 'known_calls':[e for e in branches if e['kind']=='CALL'], 'branches':branches,
              'indirect_control_flow':[i for i in cfg['indirect_sites'] if start<=i['offset']<end],
              'probable_tables':[t for t in cfg['probable_tables'] if start<=t['start']<end],
              'classification':next((r['classification'] for r in cfg.get('boundary_regions',[]) if r['start']<=start<end<=r['end']), 'CODE' if code else 'UNREACHABLE_OR_MIXED'), 'neighbors':neighbors,
              'instructions':[i for i in ins.values() if start<=i['offset']<end],
              'known_labels':[l for l in measurement.get('labels',[]) if start<=l['offset']<=end],
              'applicable_rules':['ABSOLUTE_MEMORY','BRANCH_WIDTH','BOUNDARY','QUALITY','PUBLICS','FRESHNESS'],
              'expected_fixups':[f for f in measurement['fixups'] if start<=f['offset']<end],
              'blocked_attempts':[f['id'] for f in overlapping],
              'expected_publics':[p for p in measurement['publics'] if start<=p['offset']<end],
              'known_globals':[{'instruction':i['offset'],'offset':m['displacement'], 'symbols':sorted(globals_[m['displacement']]),
                                'addressing':'indexed' if m['base'] or m['index'] else 'absolute'}
                               for i in ins.values() if start<=i['offset']<end for m in i.get('memory_references',[])
                               if m['segment']=='ds' and m['displacement'] in globals_],
              'global_resolution':'Exact DGROUP displacement bindings; indexed references identify a base, not a proven effective address.',
              'rules':'docs/current/tasm-reconstruction-rules.md',
              'verification_command':f'python tools/check_candidate.py {ident}',
              'acceptance_command':f'python tools/check_candidate.py {ident} --promote',
              'acceptance_required':True,'promotion_blocker':reason,
              'source_sha256':measurement['source_sha256'],
              'recommended_task':'Symbolic TASM transcription' if code else 'Supervisor: establish roots or data evidence'}
        if card['classification']=='UNROLLED_CODE':
            card['transcription_guidance']={
                'rule':'UNROLLED_TRANSFER', 'body_bytes':15 if start<0x7b7 else 21,
                'instruction_count':len(card['instructions']),
                'instructions':'Use the pinned tested pattern in docs/current/tasm-reconstruction-rules.md. Transcribe only this card; preserve all partial bodies after earlier promotions.'}
        cards.append(card)
    return sorted(cards,key=lambda c:(-c['priority'],c['id']))


def refresh(root=ROOT, accepted=False):
    current=root/'docs/current'; current.mkdir(exist_ok=True)
    if accepted:
        receiptpath=root/'build/exe-build-report.json'
        proof=read_json(receiptpath) if receiptpath.exists() else {}
        if (proof.get('status')!='BUILT' or proof.get('mode')!='ACCEPTANCE' or not proof.get('fresh_build')
                or proof.get('input_fingerprint')!=input_fingerprint(root)):
            raise ValueError('Accepted refresh requires a fresh complete acceptance receipt for these exact inputs')
    measurement,blob=exact_measure(root)
    oldpath=current/'runtime-progress.json'
    if oldpath.exists() and not accepted:
        old=read_json(oldpath)
        if old.get('source_sha256') != measurement['source_sha256']:
            raise ValueError('Source changed since queue generation. Run the card FAST/promotion command; refresh cannot bless an edit.')
    spec=read_json(root/'recipes/runtime/oracle.json')
    cfg=runtime_cfg(blob,root)
    metrics=quality(measurement,cfg,spec)
    cfg['already_symbolic_ranges']=[{'start':r['start'],'end':r['end'],'category':r['category']} for r in measurement['ranges'] if r['category']!='raw_unresolved']
    cfg['still_raw_ranges']=[{'start':r['start'],'end':r['end']} for r in measurement['ranges'] if r['category']=='raw_unresolved']
    # A root is closed only when its reachable instructions are non-raw and every indirect edge is resolved.
    closed=[]
    rawbytes={b for r in measurement['ranges'] if r['category']=='raw_unresolved' for b in range(r['start'],r['end'])}
    for r in cfg['roots']:
        blocks=[b for b in cfg['basic_blocks'] if set(r['names']) & set(b['roots'])]
        covered={i for b in blocks for i in b['instructions']}
        if covered and not any(i['offset'] in covered and any(b in rawbytes for b in range(i['offset'],i['offset']+i['size'])) for i in cfg['instructions']) and not any(i['offset'] in covered and not i['complete'] for i in cfg['indirect_sites']):
            closed.append(r)
    metrics.update(closed_entry_roots=len(closed),total_entry_roots=len(cfg['roots']))
    statepath=root/'recipes/runtime/task-state.json'
    state=read_json(statepath) if statepath.exists() else {}
    cards=candidates(measurement,cfg,state)
    fingerprint=input_fingerprint(root)
    report={**measurement,'source_snapshot':(root/'asm/RUNTIME_BLOCK.ASM').read_text(),'metrics':metrics,'input_fingerprint':fingerprint,'closed_roots':closed,
            'source_snapshot_base64':base64.b64encode((root/'asm/RUNTIME_BLOCK.ASM').read_bytes()).decode('ascii')}
    report.pop('object')
    carddir=current/'candidates'; carddir.mkdir(exist_ok=True)
    keep=set()
    for card in cards:
        filename=card['id'].replace(':','_')+'.json'; keep.add(filename)
        card['card']='docs/current/candidates/'+filename
        card['input_fingerprint']=fingerprint
        write_json(carddir/filename,card)
        if not (root/'tools/check_candidate.py').is_file() or not (root/card['card']).is_file():
            raise ValueError('Generated queue must reference an existing checker and candidate card')
    # Delete only obsolete generated candidate JSONs inside this exact directory.
    for path in carddir.glob('RUNTIME_BLOCK_*.json'):
        if path.name not in keep: path.unlink()
    queue=[{k:c[k] for k in ('id','priority','kind','difficulty','risk','range','bytes','source','known_roots',
                             'dependencies','verification_command','acceptance_command','acceptance_required','card','promotion_blocker')} for c in cards]
    write_json(current/'runtime-progress.json',report)
    write_json(current/'runtime-cfg.json',cfg)
    write_json(current/'grinder-queue.json',{'format':'empires-grinder-queue-v1','input_fingerprint':fingerprint,
                                           'selection':'Highest priority CHEAP task only; skip SUPERVISOR', 'tasks':queue})
    from report_source_quality import report as owner_quality
    write_json(current/'source-quality.json',{'owner_level_comparison':owner_quality(read_json(root/'layout/manifest.json')), 'format':'empires-source-quality-ranges-v1','runtime':metrics,
                                             'scope':'Runtime emission is assembler-measured; other owners retain the legacy syntactic census.'})
    interfaces=census(root)
    write_json(current/'interface-conflicts.json',interfaces)
    write_json(current/'blockers.json',{'tasks':[c for c in queue if c['difficulty']=='SUPERVISOR'], 'failure_memory':state,
        'control_flow_obligations':[i for i in cfg['indirect_sites'] if not i['complete']],
        'obligation_scope':'Unproven argument/context domains remain visible even when the instruction is symbolically recovered.'})
    receiptpath=root/'build/exe-build-report.json'
    receipt=read_json(receiptpath) if receiptpath.exists() else {}
    status={'format':'empires-factory-status-v1','input_fingerprint':fingerprint,
            'acceptance_current':receipt.get('input_fingerprint')==fingerprint and receipt.get('mode')=='ACCEPTANCE',
            'acceptance':{k:receipt.get(k) for k in ('status','mode','sha256','relocations','link_invocations','bss')},
            'structural':{'raw_exe_fallback_bytes':sum(o['end']-o['start'] for o in read_json(root/'layout/manifest.json')['regions'] if o['kind']=='RAW'),
                          'unresolved_symbols':receipt.get('unresolved_symbols'),
                          'remaining_structural_adapters':receipt.get('remaining_structural_adapters')},
            'interfaces':{'function_symbols':len(interfaces['functions']),'global_symbols':len(interfaces['globals']),
                          'conflicts_for_review':len(interfaces['conflicts']),'proven_field_layout_equivalence_groups':len(interfaces['consolidation_candidates']),
                          'unsupported_declarations':len(interfaces['unsupported'])},
            'runtime':metrics,'queue_counts':dict(Counter(c['difficulty'] for c in cards)),
            'next_task':next((c['id'] for c in cards if c['difficulty']=='CHEAP'),None),
            'authority':'Only docs/current is current status. Other checkpoints are historical evidence.'}
    write_json(current/'status.json',status)
    print(json.dumps({'runtime':metrics,'queue':status['queue_counts'],'next':status['next_task']},indent=2))
    return status


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__); parser.add_argument('command',choices=['refresh','next'],nargs='?',default='refresh'); args=parser.parse_args()
    if args.command=='next':
        queue=read_json(ROOT/'docs/current/grinder-queue.json')
        if queue['input_fingerprint']!=input_fingerprint(): raise SystemExit('STALE queue: python tools/reconstruction_factory.py refresh')
        task=next((t for t in queue['tasks'] if t['difficulty']=='CHEAP'),None)
        print(json.dumps(task,indent=2))
    else: refresh()
