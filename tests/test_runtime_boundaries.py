"""Consumer-derived table boundaries and safe grinder frontier."""
import sys
import unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from runtime_source import oracle_bytes
from runtime_cfg import runtime_cfg
from runtime_boundaries import recover


class BoundaryTests(unittest.TestCase):
    def test_table_symbols_follow_assembled_instruction_labels(self):
        from reconstruction_factory import exact_measure
        from reconstruct import ROOT
        measurement, binary = exact_measure()
        labels=[x for x in measurement['labels'] if x['name'].startswith('rt_')]
        # 184 boundary labels plus the 41 branch-target labels that replaced the
        # label-less `jcc $+N` spellings (2026-09-21); every rt_XXXX still sits at its offset.
        self.assertEqual(len(labels),225)
        self.assertTrue(all(x['offset']==int(x['name'][3:],16) for x in labels))
        self.assertEqual(measurement['fixups'],[])
        text=(ROOT/'asm/RUNTIME_BLOCK.ASM').read_text()
        self.assertNotIn('runtime_cs_',text)
        self.assertNotIn('+424h',text)
        self.assertEqual(text.lower().count('equ 039ch'),1)
        for literal in ('cs:[bx+3d8h]','cs:[bx+3e2h]','cs:[bx+3ech]','cs:[bx+190ch]','cs:[bx+191eh]','add ax,0b61h'):
            self.assertNotIn(literal,text.lower())
        self.assertEqual(binary,oracle_bytes())

    def test_tables_exclude_code_and_cover_exact_extents(self):
        b=oracle_bytes(); tables,sites,regions=recover(b); c=runtime_cfg(b)
        self.assertEqual(sum(t['end']-t['start'] for t in tables),662)
        self.assertEqual(c['issues'],[])
        code={p for i in c['instructions'] for p in range(i['offset'],i['offset']+i['size'])}
        data={p for t in tables for p in range(t['start'],t['end'])}
        self.assertFalse(code & data)
        self.assertTrue(set(range(0x307,0xf3f))<=code)
        self.assertTrue(set(range(0x15ef,0x15fc))<=code)
        self.assertEqual(len(code),5908)

    def test_computed_entries_and_patched_branches_share_targets(self):
        b=oracle_bytes(); tables,sites,_=recover(b); c=runtime_cfg(b)
        self.assertEqual(sites[str(0x305)]['targets'],list(range(0x307,0x7b7,15)))
        self.assertEqual(sites[str(0x89d)]['targets'],list(range(0x89f,0xf2f,21))+[0xf37])
        for jump,patch in [(0x305,0x7c2),(0x89d,0xf3c)]:
            self.assertEqual(sites[str(jump)]['targets'],sites[str(patch)]['targets'])
            edges=[e['target'] for e in c['edges'] if e['source']==patch]
            self.assertEqual(edges,sites[str(patch)]['targets'])
        self.assertEqual(sorted(s['offset'] for s in c['indirect_sites'] if s['complete']),[0x148f,0x1503,0x18b3])

    def test_byte_width_dispatch_bound_for_all_inputs(self):
        b=oracle_bytes(); _,sites,_=recover(b); proof=sites[str(0x18b3)]
        indices=set()
        for width in range(256):
            bx=width*4
            while bx>=16: bx-=16
            indices.add(bx)
        self.assertEqual(sorted(indices),proof['index_byte_offsets'])
        self.assertEqual(proof['targets'],sorted({int.from_bytes(b[0x1570+i:0x1572+i],'little')-0x39c for i in indices}))

    def test_compositor_byte_width_bounds(self):
        _,sites,_=recover(oracle_bytes())
        seen=set()
        for width in range(256):
            bx=2*width
            while bx>=16: bx-=16
            seen.add(bx)
        for site in (0x148f,0x1503):
            self.assertTrue(sites[str(site)]['complete'])
            self.assertEqual(sorted(seen),sites[str(site)]['index_byte_offsets'])

    def test_mask_matches_every_possible_xlat_index(self):
        b=oracle_bytes()
        for v in range(256):
            self.assertEqual(b[0x15fc+v],(0xf0 if v>>4==0 else 0)|(15 if v&15==0 else 0))

    def test_translated_table_does_not_invent_a_consumer_or_root(self):
        binary=oracle_bytes(); tables,sites,_=recover(binary); cfg=runtime_cfg(binary)
        table=next(t for t in tables if t['start']==0x191c)
        self.assertEqual(table['consumers'],[])
        self.assertEqual(table['values'],[v+0x424 for v in tables[4]['values']])
        starts={i['offset'] for i in cfg['instructions']}
        self.assertTrue(set(table['targets'])<=starts)
        self.assertFalse(set(table['targets']) & set(cfg['proven_indirect_roots']))
        self.assertEqual(sites[str(0x18b3)]['table'],'runtime_even_pixel_dispatch')
        self.assertEqual(binary[0x18b3:0x18b8].hex(),'2effa70c19')

    def test_string_does_not_classify_adjacent_ret_as_a_root(self):
        cfg=runtime_cfg(oracle_bytes())
        unknown=[g for g in cfg['unreachable_gaps'] if g['classification']=='UNREACHABLE_UNKNOWN']
        self.assertEqual(unknown,[{'start':0x19a2,'end':0x19a3,'classification':'UNREACHABLE_UNKNOWN'}])
        self.assertIn({'start':0x19a3,'end':0x19ab,'classification':'STRING'},cfg['unreachable_gaps'])
        self.assertNotIn(0x19a2,[r['offset'] for r in cfg['roots']])

    def test_changed_oracle_rejects_boundary_evidence(self):
        b=bytearray(oracle_bytes());b[0x50]^=1
        with self.assertRaises(AssertionError): recover(b)


if __name__=='__main__': unittest.main()
