import unittest
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from reconstruct import read_json


class RawDataWave136Tests(unittest.TestCase):
    def test_pointer_table_is_explicit_and_exact(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave136.json')
        evidence = read_json(ROOT / 'docs/matching-wave136-evidence.json')
        self.assertEqual(len(recipe['owners']), 1)
        owner = recipe['owners'][0]
        self.assertEqual(owner['end'] - owner['start'], 56)
        self.assertEqual(evidence['owners'][0]['bytes'], 56)
        source = read_json(ROOT / owner['source'])
        self.assertEqual(len(source['values']), 28)
        self.assertEqual(source['values'], sorted(source['values']))
        self.assertTrue(all(a < b for a, b in zip(source['values'], source['values'][1:])))
        current = next(r for r in manifest['regions'] if r['id'] == owner['id'])
        self.assertEqual(current['kind'], 'EXACT_DATA')
        self.assertEqual(current['build']['encoder'], 'u16le-table-v1')
        self.assertEqual(evidence['owners'][0]['status'], 'EQUAL')


if __name__ == '__main__':
    unittest.main()
