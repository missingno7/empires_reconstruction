"""Local evidence for graphics overlays, audio paths, and ASM origin review.

This is a research inventory, not an additional promotion queue or a claim that
current source language identifies original authorship.
"""
from reconstruct import ROOT, read_json, write_json, sha
from resource_codecs import decode_payload, encode_payload, pack_rle
from compression_source import decode_source, encode_source
from overlay_boundaries import recover


def report(root=ROOT):
    layout=read_json(root/'layout/archives/AE000.json')
    archive=(root/layout['original']['path']).read_bytes()
    if sha(archive)!=layout['original']['sha256']: raise ValueError('Archive identity differs')
    overlays=[]
    for index,mode in [(2,5),(3,2)]:
        entry=layout['resources'][index]
        block=archive[entry['start']:entry['end']]
        binary=decode_payload(block[2:],block[1])
        if len(binary)>6571: raise ValueError('Overlay exceeds reserved runtime')
        # Loader targets the same 20 three-byte ABI slots. Short jumps carry
        # NOP padding, which is not promoted to code merely by adjacency.
        roots=[{'offset':i,'names':[f'overlay_entry_{i:02x}']} for i in range(0,60,3)]
        if not all(binary[i] in (0xe9,0xeb) for i in range(0,60,3)):
            raise ValueError('Unexpected overlay veneer')
        cfg=recover(binary,entry['id'])
        write_json(root/'docs/current'/('overlay-cfg-'+entry['id']+'.json'),cfg)
        intermediate,plan=decode_source(block[2:])
        explicit_exact=pack_rle(binary)==intermediate and encode_source(pack_rle(binary),plan)==block[2:]
        if not explicit_exact: raise ValueError('Explicit overlay compression plan differs')
        plans=root/'build/overlay-compression-plans'
        plans.mkdir(parents=True,exist_ok=True)
        write_json(plans/(entry['id']+'.json'),plan)
        overlays.append({'resource':entry['id'],'display_selector':mode,'decoded_bytes':len(binary),
                         'decoded_sha256':sha(binary),'encoded_sha256':sha(block[2:]),
                         'runtime_destination_cs':0x39c,'resource_type':block[0],'flags':block[1],
                         'root_slots':roots,'entry_targets':[e for e in cfg['edges'] if e['source']<60],
                         'cfg_covered_bytes':sum(i['size'] for i in cfg['instructions']),
                         'indirect_sites':cfg['indirect_sites'],'classified_gaps':cfg['unreachable_gaps'], 'unknown_gaps':[g for g in cfg['unreachable_gaps'] if g['classification']=='UNREACHABLE_UNKNOWN'], 'typed_regions':cfg['typed_regions'],'issues':cfg['issues'],
                         'current_encoder_exact':encode_payload(binary,block[1])==block[2:],
                         'explicit_compression_plan_exact':explicit_exact,
                         'compression_plan':'build/overlay-compression-plans/'+entry['id']+'.json',
                         'queue_status':'HELD_SUPERVISOR',
                         'blocker':'Exact explicit compression plan verified; canonical archive source ownership and bounded ASM checker/promotion are still needed before grinder cards can be issued.',
                         'evidence':['src/F_48BE.C: resource 2 for selector 5; resource 3 for selector 2; memmove to f039c',
                                     'src/F_490D.C: f48be precedes f0281',
                                     'src/F_0281.C: f039c called only after replacement',
                                     'src/F_656C.C: flags decode payload; type 46h bypasses bitmap conversion']})
    platform={'format':'empires-runtime-platform-evidence-v1','scope':'Local byte/source evidence; no source-language conversion',
        'graphics':{'selectors':[
            {'value':1,'option':'E','driver':'built-in','evidence':'CS table slot 1 -> runtime 01A1; BIOS AX=000Eh and planar register writes'},
            {'value':2,'option':'C','driver':'AE000_003','evidence':'F_48BE replaces code before first runtime call'},
            {'value':3,'option':'T','driver':'built-in','evidence':'slot 3 -> runtime 0258; BIOS AX=0009h and ports 03DAh/03DEh'},
            {'value':4,'option':'M','driver':'built-in','evidence':'slot 4 -> runtime 0294; BIOS AX=0013h'},
            {'value':5,'option':'V','driver':'AE000_002','evidence':'F_48BE replaces code before first runtime call'}],
            'mapping_source':'src/STARTUP.C cmdline_parse_args keys and handlers; video_mode_select memory-driven fallback',
            'built_in_dispatch_context':[1,3,4],
            'limitation':'Context explains zero/out-of-table slots. It is not a local bounds check for arbitrary calls; built-in CFG retains incomplete status at 019C/02A2.'},
        'audio':[
            {'classification':'PC_SPEAKER_PIT_PATH','confidence':'HIGH','sources':['asm/SOUND.ASM'],
             'evidence':'Speaker gate accesses port 61h; F_C706 sends divisor low/high bytes to port 42h.'},
            {'classification':'OPL_REGISTER_AND_TIMER_PATH','confidence':'HIGH','sources':['src/OPLREG.C','asm/SOUND.ASM'],
             'evidence':'Timer registers 2/4 with status mask E0h; address/data port pair with six then 35 settling reads; voice registers A0h/B0h.',
             'limitation':'Supports an OPL-compatible path; this evidence alone does not establish OPL3 extensions.'},
            {'classification':'ALTERNATE_PACKED_NIBBLE_AUDIO_PATH','confidence':'HIGH','sources':['asm/SOUND.ASM'],
             'evidence':'Backend 1 emits packed latch/data nibbles through port C0h; alternate configuration sets port 205h.',
             'limitation':'Do not collapse these branches into the OPL path or infer exact hardware solely from existing comments.'}],
        'overlays':overlays}
    write_json(root/'docs/current/runtime-platform.json',platform)
    # Origin classifications are owned by docs/current/asm-provenance.json
    # (tools/audit_asm_provenance.py); this review only mirrors them per plan module.
    provenance={m['module']:m for m in read_json(root/'docs/current/asm-provenance.json')['modules']}
    census=[]
    for module in read_json(root/'layout/production-plan.json')['modules']:
        if module.get('tool')!='TASM.EXE': continue
        entry=provenance.get(module['id'],{})
        classification=entry.get('classification','UNDETERMINED')
        evidence=entry.get('reason_asm_remains') or 'See docs/current/asm-provenance.json.'
        census.append({'id':module['id'],'source':module['source'],'bytes':module['end']-module['start'],
                       'members':module['members'],'classification':classification,'confidence':entry.get('confidence'),
                       'evidence':evidence,'grinder_action':'Keep exact ASM; portable replacement follows the historical semantics.'})
    write_json(root/'docs/current/asm-origin-review.json',{'format':'empires-asm-origin-review-v1','modules':census,
        'scope':'Representation versus origin triage. User-authorized exact C recovery is recorded separately; source provenance remains unproved.',
        'exact_c_recoveries':[{'id':m['id'],'source':m.get('source') or ','.join(m.get('sources',[])),'bytes':m['end']-m['start'],
                               'evidence':m['provenance']['c_recovery'],'original_language_proven':False}
                              for m in read_json(root/'layout/production-plan.json')['modules']
                              if m.get('provenance',{}).get('c_recovery')]})
    return {'overlay_decoded_bytes':sum(o['decoded_bytes'] for o in overlays),
            'overlays':[{k:o[k] for k in ('resource','decoded_bytes','cfg_covered_bytes','current_encoder_exact','explicit_compression_plan_exact','issues')} for o in overlays],
            'asm_modules_reviewed':len(census)}

if __name__=='__main__':
    import json
    print(json.dumps(report(),indent=2))
