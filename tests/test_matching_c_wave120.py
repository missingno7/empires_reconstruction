import unittest
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from reconstruct import read_json


class MatchingCWave120Tests(unittest.TestCase):
    def test_fcaf1_is_c_owned_with_fourteen_fixups(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave120.json')
        owner = next(r for r in manifest['regions'] if r['id'] == 'F_CAF1')
        self.assertEqual(owner['kind'], 'MATCHING_C')
        self.assertEqual(owner['source'], 'src/F_CAF1.C')
        self.assertEqual(owner['end'] - owner['start'], 87)
        self.assertEqual(recipe['conversions'][0]['id'], 'F_CAF1')
        evidence = read_json(ROOT / 'docs/matching-wave120-evidence.json')
        self.assertEqual(evidence['matched_sha256'], owner['expected_sha256'])
        self.assertEqual(len(evidence['fixups']), 14)
        self.assertEqual(evidence['load_relocations'], [])


if __name__ == '__main__':
    unittest.main()
