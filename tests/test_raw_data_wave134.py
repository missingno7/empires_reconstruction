import unittest
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from reconstruct import read_json


class RawDataWave134Tests(unittest.TestCase):
    def test_text_and_zero_runs_are_exact(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave134.json')
        evidence = read_json(ROOT / 'docs/matching-wave134-evidence.json')
        self.assertEqual(len(recipe['owners']), 7)
        self.assertEqual(sum(o['end'] - o['start'] for o in recipe['owners']), 99)
        self.assertEqual(sum(item['bytes'] for item in evidence['owners']), 99)
        for candidate in recipe['owners']:
            owner = next(o for o in manifest['regions'] if o['id'] == candidate['id'])
            self.assertEqual(owner['kind'], 'EXACT_DATA')
            self.assertEqual(owner['build']['encoder'], candidate['build']['encoder'])
            item = next(i for i in evidence['owners'] if i['id'] == candidate['id'])
            self.assertEqual(item['sha256'], owner['expected_sha256'])
            self.assertEqual(item['status'], 'EQUAL')


if __name__ == '__main__':
    unittest.main()
