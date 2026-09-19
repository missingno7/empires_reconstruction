import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from exe_data import encode_data
from reconstruct import mismatch, read_json


class MatchingCWave76DataTests(unittest.TestCase):
    def test_split_tables_round_trip(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave76.json')
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        ids = ['DATA_10ED1', 'PAD_10EDD', 'DATA_10EE0']
        for ident in ids:
            owner = next(r for r in manifest['regions'] if r['id'] == ident)
            self.assertEqual(next(r for r in recipe['owners'] if r['id'] == ident), owner)
            data = encode_data(read_json(ROOT / owner['source']), owner['build']['encoder'])
            mismatch(original[owner['start']:owner['end']], data, owner)
        ptrs = read_json(ROOT / 'src/data/DATA_10ED1.json')['values']
        self.assertEqual(ptrs, [39025, 39055, 39115, 39176, 39330, 39266])
        rows = read_json(ROOT / 'src/data/DATA_10EE0.json')['records']
        self.assertEqual(sorted(bytes.fromhex(rows[0])), list(range(16)))
        self.assertEqual(sorted(bytes.fromhex(rows[1])), list(range(16)))


if __name__ == '__main__':
    unittest.main()
