import unittest
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from reconstruct import read_json
from exe_data import encode_data


class RawDataWave140Tests(unittest.TestCase):
    def test_owner_start_zero_runs_are_exact(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave140.json')
        evidence = read_json(ROOT / 'docs/matching-wave140-evidence.json')
        self.assertEqual(len(recipe['owners']), 2)
        self.assertEqual(sum(o['end'] - o['start'] for o in recipe['owners']), 22)
        for owner in recipe['owners']:
            source = read_json(ROOT / owner['source'])
            self.assertEqual(encode_data(source, 'zero-pad-v1'), bytes(owner['end'] - owner['start']))
            current = next(r for r in manifest['regions'] if r['id'] == owner['id'])
            self.assertEqual(current['kind'], 'EXACT_DATA')
            item = next(i for i in evidence['owners'] if i['id'] == owner['id'])
            self.assertEqual(item['status'], 'EQUAL')


if __name__ == '__main__':
    unittest.main()
