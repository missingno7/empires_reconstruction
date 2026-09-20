"""Independent checks for alternate graphics runtime ownership."""
import sys, unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from reconstruct import ROOT,read_json
from resource_codecs import decode_payload
from overlay_boundaries import recover

class OverlayTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        layout=read_json(ROOT/'layout/archives/AE000.json')
        archive=(ROOT/layout['original']['path']).read_bytes()
        cls.images={}
        for index in (2,3):
            entry=layout['resources'][index];block=archive[entry['start']:entry['end']]
            cls.images[entry['id']]=decode_payload(block[2:],block[1])

    def test_every_byte_has_disjoint_ownership(self):
        for name,binary in self.images.items():
            cfg=recover(binary,name);owned=[]
            for i in cfg['instructions']:owned.extend(range(i['offset'],i['offset']+i['size']))
            for r in cfg['typed_regions']:owned.extend(range(r['start'],r['end']))
            self.assertEqual(sorted(owned),list(range(len(binary))))
            self.assertFalse(cfg['issues'])
            for r in cfg['typed_regions']:
                if r['classification']=='PADDING':
                    # Every padding byte follows a decoded unconditional jump.
                    self.assertTrue(any(i['offset']+i['size']==r['start'] and i['mnemonic']=='jmp' for i in cfg['instructions']))

    def test_dispatch_index_domain_and_destinations(self):
        cfg=recover(self.images['AE000_003'],'AE000_003')
        indices=set()
        # AND erases all width bits except bit 1; SHR carry uses coordinate bit 1.
        for width in range(65536):
            for source_bit in (0,1):
                for destination_bit in (0,1):
                    bx=(width>>1)&1
                    bx=(bx<<1)|source_bit
                    bx=(bx<<1)|destination_bit
                    indices.add(bx*2)
        self.assertEqual(indices,set(range(0,16,2)))
        site=cfg['indirect_sites'][0]
        self.assertTrue(site['complete'])
        self.assertEqual(site['targets'],[0x32d,0x345,0x369,0x38f,0x3c1,0x3fa])
        entries={i['offset'] for i in cfg['instructions']}
        self.assertTrue(set(site['targets'])<=entries)

    def test_changed_image_rejects_old_proof(self):
        for name,binary in self.images.items():
            changed=bytearray(binary);changed[-1]^=1
            with self.assertRaises(ValueError):recover(changed,name)

if __name__=='__main__':unittest.main()
