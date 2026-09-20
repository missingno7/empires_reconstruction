"""Byte-pinned boundaries for the two graphics replacement images."""
import hashlib
from runtime_cfg import recursive_cfg

HASHES = {
    'AE000_002': '763d28e6049dee36e7be92a10201e64b0bf7d1f62d73d83736e215073e2ddf87',
    'AE000_003': '180107494d1c12406cffe25a00e76324c6ee480791873d375c311b6acadf776a',
}


def recover(binary, resource):
    if hashlib.sha256(binary).hexdigest() != HASHES[resource]:
        raise ValueError('Overlay identity differs from boundary evidence')
    roots = [{'offset': i, 'names': [f'overlay_entry_{i:02x}']} for i in range(0, 60, 3)]
    data, sites = [], {}
    def region(start, end, classification, evidence, consumers=(), **extra):
        data.append(dict(start=start, end=end, classification=classification,
                         evidence=evidence, consumers=list(consumers), confidence='HIGH', **extra))
    padding = [2, 5]
    if resource == 'AE000_003':
        targets = [int.from_bytes(binary[i:i+2], 'little') - 0x39c for i in range(0x2d2, 0x2e2, 2)]
        proof = ('02FB copies CX to BX; 02FD AND BX,1 bounds it to 0..1. '
                 '0307 and 0310 RCL BX,1 append carry bits from coordinate shifts. '
                 '0326 doubles 0..7 to even byte indices 0..14; 0328 reads CS:066E.')
        region(0x2d2, 0x2e2, 'TABLE', proof, [0x328], element_width=2,
               encoding='ABSOLUTE_CS', targets=targets, name='cga_copy_alignment_dispatch')
        sites[str(0x328)] = dict(targets=sorted(set(targets)), complete=True, evidence=proof,
                                table_range=[0x2d2, 0x2e2], byte_indices=list(range(0,16,2)))
        palette = [0,0,1,2,3,0,0,0,0,0,0,0,0,0,0,0]
        expected = bytes((palette[v >> 4] * 0x44) | (palette[v & 15] * 0x11) for v in range(256))
        if binary[0x980:0xa80] != expected:
            raise ValueError('CGA color table differs')
        expected = bytes((0xcc if v >> 4 else 0) | (0x33 if v & 15 else 0) for v in range(256))
        if binary[0xa80:0xb80] != expected:
            raise ValueError('CGA transparency mask differs')
        region(0x980, 0xa80, 'TABLE',
               'BX=0D1C at 0BA1/0CEF; CS XLAT uses the full AL byte. Each nibble maps through [0,0,1,2,3,0,...], expanded to repeated two-bit pixels.',
               [0xba1,0xcef], element_width=1, name='cga_color_pair_lookup')
        region(0xa80, 0xb80, 'TABLE',
               '0CFF INC BH selects the next 256-byte table for CS XLAT at 0D01; 0D03 DEC BH restores it. Mask=(high_nibble_nonzero?CC:0)|(low_nibble_nonzero?33:0).',
               [0xcff,0xd01,0xd03], element_width=1, name='cga_transparency_mask')
        padding += [0x3c0,0x3f9,0x439,0xceb,0xe10]
    for off in padding:
        if binary[off] != 0x90: raise ValueError('Expected skipped NOP')
        region(off, off+1, 'PADDING', '90 byte skipped by established unconditional branch; not an entry root.')
    cfg = recursive_cfg(binary, roots, base=0x39c, data_ranges=data, indirect_targets=sites)
    if cfg['issues']: raise ValueError(cfg['issues'])
    for gap in cfg['unreachable_gaps']:
        owners=[d for d in data if d['start'] == gap['start'] and d['end'] == gap['end']]
        # Adjacent lookup tables form one unreachable span.
        if not owners and gap['start']==0x980 and gap['end']==0xb80:
            owners=[d for d in data if 0x980<=d['start']<0xb80]
        if sum(d['end']-d['start'] for d in owners) != gap['end']-gap['start']:
            raise ValueError('Unowned overlay gap')
        gap['classification'] = owners[0]['classification']
    cfg['typed_regions'] = sorted(data, key=lambda d:d['start'])
    cfg['proven_indirect_roots'] = sorted({t for s in sites.values() for t in s['targets']})
    cfg['work_units'] = [dict(range=[b['start'],b['end']], classification='CODE',
        entry_roots=b['roots'], targets=[e['target'] for e in cfg['edges'] if b['start']<=e['source']<b['end'] and e['kind']!='FALLTHROUGH'],
        evidence='Recursive CFG from loader ABI entries and proven dispatch targets.', confidence='HIGH',
        queue_status='HELD_ARCHIVE_SOURCE_INTEGRATION') for b in cfg['basic_blocks']]
    return cfg
