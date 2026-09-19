import copy
import json
from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from pointer_records import compile_records, bind_records, records_object
from omf import OmfReader
from mz import MZ


class PointerRecordTests(unittest.TestCase):
    def test_four_record_table_has_one_null_pointer(self):
        manifest = json.loads((ROOT / 'layout/manifest.json').read_text())
        owners = {o['id']: o for o in manifest['regions']}
        owner = owners['DATA_010FA5_RECORDS']
        doc = json.loads((ROOT / owner['source']).read_text())
        data, refs = compile_records(doc)
        self.assertEqual((len(data), len(refs)), (80, 7))
        self.assertIsNone(doc['records'][1]['pointer_a'])
        self.assertEqual(data[22:26], bytes(4))
        frame = manifest['frames']['DGROUP']
        bound = bind_records(doc, lambda target: (owners[target]['start'] - 512 - frame, frame // 16))
        self.assertEqual(bound, (ROOT / 'assets/AEPROG.EXE').read_bytes()[owner['start']:owner['end']])

    def test_source_bytes_and_relocations(self):
        manifest = json.loads((ROOT / 'layout/manifest.json').read_text())
        owners = {o['id']: o for o in manifest['regions']}
        owner = owners['DATA_011D90_RECORDS']
        doc = json.loads((ROOT / owner['source']).read_text())
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        frame = manifest['frames']['DGROUP']
        resolve = lambda target: (owners[target]['start'] - 512 - frame, frame // 16)
        bound = bind_records(doc, resolve)
        self.assertEqual(bound, original[owner['start']:owner['end']])
        payload, refs = compile_records(doc)
        module = OmfReader().read(records_object(doc, owner['id']))
        self.assertEqual(module.segment_bytes('_DATA'), payload)
        fixups = module.fixups_in('_DATA')
        self.assertEqual([(f['offset'], f['target'], f['loc']) for f in fixups],
                         [(r['offset'], r['target'], 'pointer32') for r in refs])
        sites = [r['load_offset'] for r in MZ.parse(original).relocations
                 if owner['start'] <= r['load_offset'] + 512 < owner['end']]
        self.assertEqual(sorted(sites), [owner['start'] - 512 + r['offset'] + 2 for r in refs])
        edited = copy.deepcopy(doc)
        edited['records'][0]['byte_a'] ^= 1
        changed = bind_records(edited, resolve)
        self.assertEqual([i for i, (a, b) in enumerate(zip(bound, changed)) if a != b], [6])


if __name__ == '__main__':
    unittest.main()
