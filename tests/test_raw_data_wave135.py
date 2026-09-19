import unittest
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from reconstruct import read_json


class RawDataWave135Tests(unittest.TestCase):
    def test_zero_initialized_region_is_exact(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave135.json')
        evidence = read_json(ROOT / 'docs/matching-wave135-evidence.json')
        self.assertEqual(len(recipe['owners']), 1)
        self.assertEqual(recipe['owners'][0]['end'] - recipe['owners'][0]['start'], 3155)
        self.assertEqual(evidence['owners'][0]['bytes'], 3155)
        owner = next(o for o in manifest['regions'] if o['id'] == recipe['owners'][0]['id'])
        self.assertEqual(owner['kind'], 'EXACT_DATA')
        self.assertEqual(owner['build']['encoder'], 'zero-pad-v1')
        self.assertEqual(evidence['owners'][0]['status'], 'EQUAL')


if __name__ == '__main__':
    unittest.main()
