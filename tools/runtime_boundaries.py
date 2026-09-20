"""Byte-pinned, consumer-derived runtime table and mutable-branch evidence.

All coordinates are owner-relative and half-open. Index domains are documented
preconditions, not inferred from plausible instruction decoding.
"""
import hashlib


def recover(binary):
    assert hashlib.sha256(binary).hexdigest() == '1a3f517688ed204f52e8d9991ea7098ce138d53880cb2692003af0fb70561783'
    tables, sites = [], {}
    def table(name, start, end, width, consumers, encoding, evidence, anchor=None):
        values = [int.from_bytes(binary[p:p+width], 'little', signed=anchor is not None)
                  for p in range(start, end, width)]
        targets = sorted(set(anchor + v if anchor is not None else v-0x39c
                             for v in values if anchor is not None or v)) if width == 2 else []
        row = dict(name=name, start=start, end=end, range=[start,end], classification='TABLE',
                   element_width=width, encoding=encoding, consumers=consumers, targets=targets,
                   entry_roots=[], evidence=evidence, confidence='HIGH', values=values)
        if anchor is not None: row['anchor']=anchor
        tables.append(row)
        return targets
    a=table('runtime_mode_dispatch',0x3c,0x46,2,[0x19c],'absolute CS offsets; zero unsupported slots',
            '0194 clears BH, loads display selector, doubles BX; 019C indexes CS:03D8. Five stored slots; selector is not locally bounded.')
    b=table('runtime_blit_dispatch',0x46,0x50,2,[0x2a2],'absolute CS offsets; zero unsupported slots',
            '029A clears BH, loads display selector, doubles BX; 02A2 indexes CS:03E2. Mode 5 spills into the following table and remains unproven.')
    c=table('runtime_planar_entry_deltas',0x50,0xf2,2,[0x2f8,0x2fd,0x301,0x305,0x7c2],'signed relative entry / rel16 patch',
            '02F8 reads CS:03EC+2*width; 02FD writes the rel16 at CS:0B5F; 0301 adds CS:0B61. 81 words: indices 0/1 alias, then 15-byte strides. Valid stored domain 0..80; caller width bounds remain unproven.',0x7c5)
    d=table('runtime_packed_entry_deltas',0xf2,0x194,2,[0x87b,0x880,0x89a,0x89d,0xf3c],'signed relative entry / rel16 patch',
            '087B reads CS:048E+2*width; 0880 patches CS:12D9; 089A adds CS:12DB. 81 words: zero selects epilogue, then 21-byte strides. Valid stored domain 0..80; caller bounds remain unproven.',0xf3f)
    e=table('runtime_even_pixel_dispatch',0x1570,0x1582,2,[0x148f,0x18b3],'absolute CS offsets',
            'Even BX pixel count indexes CS:190C after signed comparison with 16. Nine words include the full-width entry. Consumers 148F/18B3 have zero-extended byte counts; their proven index domains are recorded per site.')
    f=table('runtime_odd_pixel_dispatch',0x1582,0x1594,2,[0x1503],'absolute CS offsets',
            'Even BX pixel count indexes CS:191E; paired with table at 1570 and ends exactly at public blit target 1594.')
    table('runtime_transparency_mask',0x15fc,0x16fc,1,[0x17a3,0x17ab,0x1832,0x1844],'byte mask indexed by source pixel pair',
          'BX=CS:1998 followed by CS XLAT at 17AB/1844; AL is an 8-bit index. For each value v, mask=(F0 if high nibble is zero else 0)|(0F if low nibble is zero else 0). RET at 15FB; next public root 16FC.')
    assert binary[0x15fc:0x16fc] == bytes((0xf0 if v<16 else 0)|(15 if v%16==0 else 0) for v in range(256))
    for offsets, targets, name in [([0x19c],a,tables[0]['name']),([0x2a2],b,tables[1]['name']),
                                  ([0x305,0x7c2],c,tables[2]['name']),([0x89d,0xf3c],d,tables[3]['name']),
                                  ([0x148f,0x18b3],e,tables[4]['name']),([0x1503],f,tables[5]['name'])]:
        for at in offsets:
            sites[str(at)]={'targets':targets,'complete':False,'table':name,
                            'evidence':'All stored valid-domain entries; unchecked index precondition remains open.',
                            'mutable_direct':at in (0x7c2,0xf3c)}
    # This consumer has a local bound independent of caller-supplied words:
    # XOR BX,BX; MOV BL,ES:[SI]; SHL BX,1 twice. The positive loop subtracts
    # 16, so the indirect path's signed BX<16 is exactly 0,4,8,12.
    indices=[0,4,8,12]
    sites[str(0x18b3)]={
        'targets':sorted({int.from_bytes(binary[0x1570+i:0x1572+i],'little')-0x39c for i in indices}),
        'complete':True, 'table':tables[4]['name'], 'index_byte_offsets':indices,
        'evidence':'1889 XOR BX,BX; 188D MOV BL,ES:[SI]; 1899/189B SHL BX,1 => 0..1020 in multiples of four. 18A9/18AC select BX<16; 1909 subtracts 16 before positive repeat at 190C. No wrapping or negative index reaches 18B3.',
        'mutable_direct':False}
    for at,table_index in [(0x148f,4),(0x1503,5)]:
        indices=list(range(0,16,2))
        t=tables[table_index]
        sites[str(at)]={
            'targets':sorted({int.from_bytes(binary[t['start']+i:t['start']+i+2],'little')-0x39c for i in indices}),
            'complete':True, 'table':t['name'], 'index_byte_offsets':indices,
            'evidence':'141E loads CL from a byte; 1421 clears CH; 145E copies CX to BX; 1460 doubles BX => even 0..510. Each positive block iteration subtracts 16 before repeating. Signed BX<16 therefore selects only even 0..14. Shared entry from 18B3 supplies bounded multiples of four (0..1020), preserving the same even remainder domain.',
            'mutable_direct':False}
    for at in (0x19c,0x2a2):
        sites[str(at)]['execution_context']={
            'image':'built-in runtime','normal_selectors':[1,3,4],
            'replaced_selectors':{'2':'AE000_003','5':'AE000_002'},
            'evidence':'F_490D calls F_48BE before F_0281. F_48BE copies resource 2 or 3 over CS:039C for selectors 5 or 2. Arbitrary direct entry still lacks a local bounds check.'}
    regions=[]
    for start,end,kind,roots,deps,why in [
        (0x307,0x7b7,'UNROLLED_CODE',c,[tables[2]['name']],'80 repetitions of 15-byte pixel transfer; falls into row tail'),
        (0x7b7,0x7c2,'CODE',[0x7b7],[tables[2]['name']],'row strides; LOOP to mutable jump 07C2; fallthrough restores DS/DI/SI/BP and RET'),
        (0x7c2,0x7c5,'CODE',[0x7c2],[tables[2]['name']],'self-modified rel16 jump, written at 02FD'),
        (0x7c5,0x83b,'CODE',[0x7c5],[tables[1]['name']],'dispatch slot 3; bounded copy routine with loops and RET'),
        (0x83b,0x89f,'CODE',[0x83b],[tables[1]['name'],tables[3]['name']],'dispatch slot 4; setup ending at computed JMP AX'),
        (0x89f,0xf2f,'UNROLLED_CODE',d,[tables[3]['name']],'80 repetitions of 21-byte pixel transform; falls into row tail'),
        (0xf2f,0xf3c,'CODE',[0xf2f,0xf37],[tables[3]['name']],'row strides; LOOP to mutable jump 0F3C; epilogue RET'),
        (0xf3c,0xf3f,'CODE',[0xf3c],[tables[3]['name']],'self-modified rel16 jump, written at 0880'),
        (0x15ef,0x15fc,'CODE',[0x15f8],[],'reachable blit tail; instruction at 15EF straddles old raw boundary 15F0; RET at 15FB')]:
        regions.append(dict(start=start,end=end,range=[start,end],classification=kind,
                            entry_roots=[r for r in roots if start<=r<end],consumers=[],targets=[],
                            dependencies=deps,evidence=why,confidence='HIGH'))
    # The trailing table has no observed consumer. Its type is established by
    # a complete relocation-shaped clone of the proven earlier dispatch table,
    # paired with byte-identical pixel operations at the same address delta.
    original=[int.from_bytes(binary[p:p+2],'little') for p in range(0x1570,0x1582,2)]
    copied=[int.from_bytes(binary[p:p+2],'little') for p in range(0x191c,0x192e,2)]
    assert copied==[v+0x424 for v in original]
    assert binary[0x1494:0x14d7]==binary[0x18b8:0x18fb]
    table('runtime_unreferenced_pixel_dispatch',0x191c,0x192e,2,[],'absolute CS offsets; unreferenced translated copy',
          'All nine words equal the proven table at 1570 plus 0424h; pixel operations 1494..14D7 and 18B8..18FB are identical at that same translation. RET at 191B and next public routine at 192E bound this table. The observed 18B3 consumer still uses CS:190C (table 1570), not this copy. No new CFG roots inferred.')
    tables[-1]['translation_evidence']={'source_table':[0x1570,0x1582],'delta':0x424,
        'source_code':[0x1494,0x14d7],'copied_code':[0x18b8,0x18fb],'observed_consumers':[],
        'finding':'Potential stale table reference or unreachable short-width path; preserve original bytes.'}
    assert binary[0x19a3:0x19ab]==b'EGA.DRV\x00'
    table('runtime_driver_name',0x19a3,0x19ab,1,[],'NUL-terminated ASCII identifier',
          'Exact bounded ASCII EGA.DRV followed by NUL at the end public. No consumer is asserted. Preceding byte 19A2 is kept unknown, not promoted to a RET root.')
    tables[-1]['classification']='STRING'
    return tables, sites, regions
