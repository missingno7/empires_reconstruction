import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from exe_data import encode_data
from reconstruct import mismatch, read_json


class MatchingCWave106DataTests(unittest.TestCase):
    def test_runtime_block_partition_round_trip(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave106.json')
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        ids = [
            'DATA_00FC23_PAD', 'DATA_00FC34', 'DATA_00FC5F',
            'DATA_00FC8C_PAD', 'DATA_00FCC2',
        ]
        self.assertEqual(sum(next(r for r in recipe['owners'] if r['id'] == ident)['end'] -
                             next(r for r in recipe['owners'] if r['id'] == ident)['start']
                             for ident in ids), 299)
        for ident in ids:
            owner = next(r for r in manifest['regions'] if r['id'] == ident)
            self.assertEqual(next(r for r in recipe['owners'] if r['id'] == ident), owner)
            data = encode_data(read_json(ROOT / owner['source']), owner['build']['encoder'])
            mismatch(original[owner['start']:owner['end']], data, owner)
        self.assertEqual(read_json(ROOT / 'src/data/DATA_00FC34.json')['text'],
                         'Turbo-C - Copyright (c) 1988 Borland Intl.')
        self.assertIn('Abnormal program termination',
                      read_json(ROOT / 'src/data/DATA_00FC5F.json')['text'])
        self.assertEqual(len(read_json(ROOT / 'src/data/DATA_00FCC2.json')['values']), 70)


if __name__ == '__main__':
    unittest.main()
