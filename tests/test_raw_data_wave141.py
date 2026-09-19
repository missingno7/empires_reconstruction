import unittest
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from reconstruct import read_json
from exe_data import encode_data


class RawDataWave141Tests(unittest.TestCase):
    def test_palette_boundary_alignment_is_exact(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave141.json')
        evidence = read_json(ROOT / 'docs/matching-wave141-evidence.json')
        self.assertEqual(len(recipe['owners']), 1)
        owner = recipe['owners'][0]
        self.assertEqual(owner['end'] - owner['start'], 6)
        source = read_json(ROOT / owner['source'])
        self.assertEqual(encode_data(source, 'zero-pad-v1'), bytes(6))
        current = next(r for r in manifest['regions'] if r['id'] == owner['id'])
        self.assertEqual(current['kind'], 'EXACT_DATA')
        item = next(i for i in evidence['owners'] if i['id'] == owner['id'])
        self.assertEqual(item['status'], 'EQUAL')


if __name__ == '__main__':
    unittest.main()
