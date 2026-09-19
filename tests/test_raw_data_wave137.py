import unittest
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from reconstruct import read_json
from exe_data import encode_data


class RawDataWave137Tests(unittest.TestCase):
    def test_pointer_attributes_are_fixed_records(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave137.json')
        evidence = read_json(ROOT / 'docs/matching-wave137-evidence.json')
        owner = recipe['owners'][0]
        source = read_json(ROOT / owner['source'])
        self.assertEqual(owner['end'] - owner['start'], 28)
        self.assertEqual(source['format'], 'fixed-records-v1')
        self.assertEqual(source['record_size'], 1)
        self.assertEqual(len(source['records']), 28)
        self.assertEqual(encode_data(source, 'fixed-records-v1'),
                         (ROOT / 'assets/AEPROG.EXE').read_bytes()[owner['start']:owner['end']])
        current = next(r for r in manifest['regions'] if r['id'] == owner['id'])
        self.assertEqual(current['kind'], 'EXACT_DATA')
        self.assertEqual(evidence['owners'][0]['status'], 'EQUAL')


if __name__ == '__main__':
    unittest.main()
