import unittest
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from exe_data import encode_data
from mz import MZ
from reconstruct import read_json


class RawDataWave144Tests(unittest.TestCase):
    def test_control_tables_are_aligned_and_relocation_free(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave144.json')
        evidence = read_json(ROOT / 'docs/matching-wave144-evidence.json')
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        mz = MZ.parse(original)
        for owner, item in zip(recipe['owners'], evidence['owners']):
            current = next(r for r in manifest['regions'] if r['id'] == owner['id'])
            self.assertEqual(current['kind'], 'EXACT_DATA')
            self.assertEqual(owner['start'] % 2, 0)
            self.assertEqual(owner['end'] - owner['start'], item['bytes'])
            self.assertEqual(encode_data(read_json(ROOT / owner['source']), 'u16le-table-v1'),
                             original[owner['start']:owner['end']])
            load_start = owner['start'] - 512
            load_end = owner['end'] - 512
            self.assertFalse(any(load_start <= r['load_offset'] < load_end
                                 for r in mz.relocations))


if __name__ == '__main__':
    unittest.main()
