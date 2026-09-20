"""Read-only handover audit: current bytes, evidence, cards and acceptance."""
import json
from collections import Counter
from reconstruct import ROOT, read_json
from factory_inputs import input_fingerprint
from reconstruction_factory import exact_measure, candidates
from runtime_cfg import runtime_cfg
from runtime_source import quality
from check_candidate import snapshot_bytes


def audit(root=ROOT):
    def require(condition, message):
        if not condition: raise ValueError(message)
    current=root/'docs/current'
    queue=read_json(current/'grinder-queue.json')
    progress=read_json(current/'runtime-progress.json')
    status=read_json(current/'status.json')
    receipt=read_json(root/'build/exe-build-report.json')
    fingerprint=input_fingerprint(root)
    for name,report in [('queue',queue),('progress',progress),('status',status),('acceptance',receipt)]:
        require(report.get('input_fingerprint')==fingerprint, name+' is stale')
    require(receipt.get('status')=='BUILT' and receipt.get('mode')=='ACCEPTANCE' and receipt.get('fresh_build'), 'Fresh acceptance required')
    measurement,binary=exact_measure(root)
    cfg=runtime_cfg(binary,root)
    quality(measurement,cfg,read_json(root/'recipes/runtime/oracle.json'))
    require(progress['source_sha256']==measurement['source_sha256'],'Source changed')
    require(snapshot_bytes(progress)==(root/'asm/RUNTIME_BLOCK.ASM').read_bytes(),'Accepted restore snapshot differs')
    require(progress['source_snapshot']==(root/'asm/RUNTIME_BLOCK.ASM').read_text(),'Source snapshot differs')
    require(not cfg['issues'],'CFG has issues')
    published=read_json(current/'runtime-cfg.json')
    require(all(published.get(k)==v for k,v in cfg.items()),'Published CFG differs from current evidence')
    statepath=root/'recipes/runtime/task-state.json'
    expected=candidates(measurement,cfg,read_json(statepath) if statepath.exists() else {})
    require([c['id'] for c in expected]==[c['id'] for c in queue['tasks']],'Queue selection/order differs')
    covered=set()
    for card,task in zip(expected,queue['tasks']):
        path=root/task['card'];require(path.is_file(),'Missing card: '+str(path))
        actual=read_json(path)
        require(all(actual.get(k)==v for k,v in card.items()),'Card differs: '+card['id'])
        require(all(actual.get(k)==v for k,v in task.items()),'Queue/card mismatch: '+card['id'])
        require(actual.get('input_fingerprint')==fingerprint,'Stale card: '+card['id'])
        require((root/'tools/check_candidate.py').is_file(),'Missing checker')
        a,b=card['range']; span=set(range(a,b))
        require(not covered & span,'Overlapping cards');covered.update(span)
        if card['difficulty']!='SUPERVISOR':
            instructions=card['instructions']
            require(instructions and instructions[0]['offset']==a and instructions[-1]['offset']+instructions[-1]['size']==b,'Card cuts an instruction')
            require(sum(i['size'] for i in instructions)==b-a,'Card has uncovered bytes')
            require(not any(not s['complete'] for s in card['indirect_control_flow']),'Unresolved indirect in grinder card')
    raw={p for r in measurement['ranges'] if r['category']=='raw_unresolved' for p in range(r['start'],r['end'])}
    require(covered==raw,'Queue does not exactly partition raw bytes')
    return {'status':'PASS','cards':len(expected),'raw_bytes':len(raw),
            'queue_bytes':dict(Counter({d:sum(c['bytes'] for c in expected if c['difficulty']==d) for d in ('CHEAP','MEDIUM','SUPERVISOR')})),
            'acceptance_sha256':receipt['sha256']}


if __name__=='__main__':
    print(json.dumps(audit(),indent=2))
