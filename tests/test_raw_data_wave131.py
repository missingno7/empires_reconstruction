import unittest
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from reconstruct import read_json


class RawDataWave131Tests(unittest.TestCase):
    def test_eight_data_components_are_explicit_and_exact(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave131.json')
        evidence = read_json(ROOT / 'docs/matching-wave131-evidence.json')
        self.assertEqual(len(recipe['owners']), 8)
        self.assertEqual(sum(o['end'] - o['start'] for o in recipe['owners']), 122)
        self.assertEqual(sum(item['bytes'] for item in evidence['owners']), 122)
        for candidate in recipe['owners']:
            owner = next(o for o in manifest['regions'] if o['id'] == candidate['id'])
            self.assertEqual(owner['kind'], 'EXACT_DATA')
            self.assertEqual(owner['source'], candidate['source'])
            item = next(i for i in evidence['owners'] if i['id'] == candidate['id'])
            self.assertEqual(item['sha256'], owner['expected_sha256'])
            self.assertEqual(item['status'], 'EQUAL')


if __name__ == '__main__':
    unittest.main()
