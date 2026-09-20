"""FAST: one runtime module. --promote: fresh complete uncached acceptance."""
import argparse
import base64
import json
import subprocess
from pathlib import Path
from reconstruct import ROOT, read_json, write_json, sha
from runtime_source import assemble_runtime, oracle_bytes, quality
from factory_inputs import input_fingerprint


class CandidateFailure(ValueError):
    def __init__(self, pattern, message, **details):
        super().__init__(message); self.pattern=pattern; self.details=details


def diagnose_bytes(expected, actual):
    at=next((i for i,(a,b) in enumerate(zip(expected,actual)) if a!=b),min(len(expected),len(actual)))
    pattern='TASM_SHORTENED_BRANCH' if at<len(expected) and at<len(actual) and expected[at]==0xe9 and actual[at]==0xeb else 'UNKNOWN_BYTE_MISMATCH'
    if at+1<len(expected) and at<len(actual) and expected[at]==0x8b and expected[at+1]&0xc7==6 and actual[at]==0xb8+((expected[at+1]>>3)&7):
        pattern='TASM_ABSOLUTE_MEMORY'
    return {'pattern':pattern,'first_mismatch':at,'expected':expected[at:at+12].hex(' '),
            'actual':actual[at:at+12].hex(' '),'expected_extent':len(expected),'actual_extent':len(actual)}


def outside_scope(before, after, first, last):
    """Line interval is inclusive; edits may add/remove lines within it."""
    old=before.splitlines(keepends=True); new=after.splitlines(keepends=True)
    prefix, suffix=old[:first-1],old[last:]
    return new[:len(prefix)]==prefix and (not suffix or new[-len(suffix):]==suffix) and len(new)>=len(prefix)+len(suffix)


def context(ident,root=ROOT):
    queue=read_json(root/'docs/current/grinder-queue.json')
    task=next((t for t in queue['tasks'] if t['id']==ident),None)
    if not task: raise CandidateFailure('STALE_INPUTS','Candidate is absent from the current queue')
    card=read_json(root/task['card']); baseline=read_json(root/'docs/current/runtime-progress.json')
    if input_fingerprint(root,baseline['source_sha256'])!=queue['input_fingerprint']:
        raise CandidateFailure('STALE_INPUTS','Non-runtime inputs changed; refresh the queue before editing')
    return card,baseline


def check(ident,root=ROOT):
    (root/'build/candidate-fast.json').unlink(missing_ok=True)
    card,baseline=context(ident,root)
    if card['difficulty']=='SUPERVISOR':
        raise CandidateFailure('SUPERVISOR_REQUIRED',card['promotion_blocker'] or 'Supervisor task')
    source=root/card['source']; current=source.read_text()
    if not outside_scope(baseline['source_snapshot'],current,*card['source_lines']):
        raise CandidateFailure('OUT_OF_SCOPE_EDIT','Source outside the selected card changed')
    fingerprint=input_fingerprint(root)
    measurement,actual=assemble_runtime(root,cache=True)
    expected=oracle_bytes(root)
    if actual!=expected:
        diagnosis=diagnose_bytes(expected,actual)
        diagnosis['candidate_offset']=diagnosis['first_mismatch']-card['range'][0]
        diagnosis['first_mismatch_hex']=hex(diagnosis['first_mismatch'])
        raise CandidateFailure(diagnosis.pop('pattern'),'Runtime bytes differ',**diagnosis)
    spec=read_json(root/'recipes/runtime/oracle.json')
    if measurement['publics']!=sorted(spec['publics'],key=lambda p:(p['offset'],p['name'])):
        raise CandidateFailure('PUBLIC_OFFSET_CHANGED','Runtime public offsets or names changed',expected=spec['publics'],actual=measurement['publics'])
    if measurement['fixups']!=spec['fixups']:
        raise CandidateFailure('FIXUP_TOPOLOGY_CHANGED','Runtime fixup sequence changed',expected=spec['fixups'],actual=measurement['fixups'])
    metrics=quality(measurement,evidence=spec)
    a,b=card['range']
    inside=[r for r in measurement['ranges'] if a<=r['start'] and r['end']<=b]
    if sum(r['end']-r['start'] for r in inside)!=b-a:
        raise CandidateFailure('OUT_OF_SCOPE_EDIT','An emission crosses the candidate boundary')
    remaining=sum(r['end']-r['start'] for r in inside if r['category']=='raw_unresolved')
    improvement=card['raw_bytes_remaining']-remaining
    if improvement<=0 or metrics['raw_unresolved_bytes']>=baseline['metrics']['raw_unresolved_bytes']:
        raise CandidateFailure('NO_QUALITY_IMPROVEMENT','Exact bytes, but unresolved bytes did not decrease')
    if fingerprint!=input_fingerprint(root):
        raise CandidateFailure('STALE_INPUTS','Inputs changed during FAST')
    result={'status':'PASS','level':'FAST','candidate':ident,'bytes_checked':len(actual),
            'publics_checked':len(measurement['publics']),'fixups_checked':len(measurement['fixups']),
            'raw_bytes_removed':improvement,'metrics':metrics,'input_fingerprint':fingerprint}
    write_json(root/'build/candidate-fast.json',result)
    return result


def record_failure(ident,error,root=ROOT):
    patterns=read_json(root/'recipes/runtime/failure-patterns.json')
    hint=next((p for p in patterns if p['pattern']==error.pattern),None)
    failure={'candidate':ident,'status':'FAIL','pattern':error.pattern,'message':str(error),
             'details':error.details,'hint':hint,'input_fingerprint':input_fingerprint(root)}
    path=root/'build/factory-failures.json'
    history=read_json(path) if path.exists() else []
    history.append(failure); write_json(path,history)
    write_json(root/'build/candidate-fast.json',failure)
    return failure


def snapshot_bytes(baseline):
    """Restore the exact accepted file, including its original newline encoding."""
    if 'source_snapshot_base64' in baseline:
        data=base64.b64decode(baseline['source_snapshot_base64'],validate=True)
        if sha(data)!=baseline['source_sha256']:
            raise CandidateFailure('STALE_INPUTS','Accepted source snapshot digest differs')
        return data
    # Older reports stored normalized text only. Accept a legacy reconstruction
    # solely when its file hash proves the original LF or CRLF representation.
    text=baseline['source_snapshot']
    choices=[text.encode('utf-8'),text.replace('\n','\r\n').encode('utf-8')]
    for data in choices:
        if sha(data)==baseline.get('source_sha256'):
            return data
    raise CandidateFailure('STALE_INPUTS','No byte-exact accepted snapshot; supervisor refresh required')


def block(ident,reason,root=ROOT):
    card,baseline=context(ident,root)
    source=root/card['source']; current=source.read_text()
    if not outside_scope(baseline['source_snapshot'],current,*card['source_lines']):
        raise CandidateFailure('OUT_OF_SCOPE_EDIT','Refusing to restore a failed candidate with unrelated edits')
    accepted_bytes=snapshot_bytes(baseline)
    archive=root/'build/blocked-candidates'; archive.mkdir(exist_ok=True)
    saved=archive/(ident.replace(':','_')+'-'+sha(source.read_bytes())[:12]+'.ASM')
    saved.write_bytes(source.read_bytes())
    failurepath=root/'build/candidate-fast.json'
    failure=read_json(failurepath) if failurepath.exists() else {'reason':reason}
    if failure.get('candidate')!=ident:
        failure={'candidate':ident,'reason':reason,'note':'No diagnostic for this candidate; unrelated last failure excluded.'}
    statepath=root/'recipes/runtime/task-state.json'; state=read_json(statepath) if statepath.exists() else {}
    state[ident]={'status':'BLOCKED_SUPERVISOR','range':card['range'],'reason':reason,'failure':failure,'saved_source':saved.relative_to(root).as_posix()}
    write_json(statepath,state)
    source.write_bytes(accepted_bytes)
    from reconstruction_factory import refresh
    refresh(root)
    return {'status':'BLOCKED_SUPERVISOR','candidate':ident,'saved_source':str(saved),'source_restored':True}


def promote(ident,root=ROOT):
    (root/'build/candidate-promotion.json').unlink(missing_ok=True)
    result=check(ident,root)
    from build_production import build
    proof=build(root,verify=True,research=False)
    if proof['input_fingerprint']!=result['input_fingerprint']:
        raise CandidateFailure('STALE_INPUTS','FAST and ACCEPTANCE inputs differ')
    from reconstruction_factory import refresh
    refresh(root,accepted=True)
    write_json(root/'build/candidate-promotion.json',{'status':'ACCEPTED','candidate':ident,
                'input_fingerprint':proof['input_fingerprint'],'sha256':proof['sha256'],
                'raw_bytes_removed':result['raw_bytes_removed']})
    return {'status':'ACCEPTED','candidate':ident,'raw_bytes_removed':result['raw_bytes_removed'],
            'sha256':proof['sha256'],'next':'Review diff, commit source and generated current reports, then select next CHEAP task.'}


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__); p.add_argument('candidate'); g=p.add_mutually_exclusive_group()
    g.add_argument('--promote',action='store_true'); g.add_argument('--block',metavar='REASON'); args=p.parse_args()
    try:
        result=block(args.candidate,args.block) if args.block else promote(args.candidate) if args.promote else check(args.candidate)
        print(json.dumps(result,indent=2))
    except (ValueError,OSError,KeyError,subprocess.SubprocessError) as error:
        if not isinstance(error,CandidateFailure):
            diagnostics=getattr(error,'diagnostics',None)
            error=CandidateFailure('ASSEMBLY_ERROR' if diagnostics else 'UNKNOWN_FAILURE',str(error),diagnostics=diagnostics)
        print(json.dumps(record_failure(args.candidate,error),indent=2))
        raise SystemExit(1)
