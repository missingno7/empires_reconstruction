import unittest
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from reconstruct import read_json


class MatchingCWave119Tests(unittest.TestCase):
    def test_fc27d_is_c_owned_with_eighteen_fixups(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave119.json')
        owner = next(r for r in manifest['regions'] if r['id'] == 'F_C27D')
        self.assertEqual(owner['kind'], 'MATCHING_C')
        self.assertEqual(owner['source'], 'recovery/src/F_C27D.C')
        self.assertEqual(owner['end'] - owner['start'], 109)
        self.assertEqual(recipe['conversions'][0]['id'], 'F_C27D')
        evidence = read_json(ROOT / 'docs/matching-wave119-evidence.json')
        self.assertEqual(evidence['matched_sha256'], owner['expected_sha256'])
        self.assertEqual(len(evidence['fixups']), 18)
        self.assertEqual(evidence['load_relocations'], [])


if __name__ == '__main__':
    unittest.main()
