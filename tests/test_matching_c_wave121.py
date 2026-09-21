import unittest
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from reconstruct import read_json


class MatchingCWave121Tests(unittest.TestCase):
    def test_fc834_is_c_owned_with_nine_fixups(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave121.json')
        owner = next(r for r in manifest['regions'] if r['id'] == 'F_C834')
        self.assertEqual(owner['kind'], 'MATCHING_C')
        self.assertEqual(owner['source'], 'recovery/src/F_C834.C')
        self.assertEqual(owner['end'] - owner['start'], 67)
        self.assertEqual(recipe['conversions'][0]['id'], 'F_C834')
        evidence = read_json(ROOT / 'docs/matching-wave121-evidence.json')
        self.assertEqual(evidence['matched_sha256'], owner['expected_sha256'])
        self.assertEqual(len(evidence['fixups']), 9)
        self.assertEqual(evidence['load_relocations'], [])


if __name__ == '__main__':
    unittest.main()
