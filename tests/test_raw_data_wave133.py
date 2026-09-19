import unittest
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from reconstruct import read_json


class RawDataWave133Tests(unittest.TestCase):
    def test_split_padding_tables_and_text_are_exact(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave133.json')
        evidence = read_json(ROOT / 'docs/matching-wave133-evidence.json')
        self.assertEqual(len(recipe['owners']), 13)
        self.assertEqual(sum(o['end'] - o['start'] for o in recipe['owners']), 126)
        self.assertEqual(sum(item['bytes'] for item in evidence['owners']), 126)
        for candidate in recipe['owners']:
            owner = next(o for o in manifest['regions'] if o['id'] == candidate['id'])
            self.assertEqual(owner['kind'], 'EXACT_DATA')
            self.assertEqual(owner['build']['encoder'], candidate['build']['encoder'])
            item = next(i for i in evidence['owners'] if i['id'] == candidate['id'])
            self.assertEqual(item['sha256'], owner['expected_sha256'])
            self.assertEqual(item['status'], 'EQUAL')


if __name__ == '__main__':
    unittest.main()
