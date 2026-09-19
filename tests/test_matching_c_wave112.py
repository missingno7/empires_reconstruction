import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from reconstruct import read_json


class MatchingCWave112Tests(unittest.TestCase):
    def test_f6b1a_is_c_owned_with_two_fixups(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave112.json')
        owner = next(r for r in manifest['regions'] if r['id'] == 'F_6B1A')
        self.assertEqual(owner['kind'], 'MATCHING_C')
        self.assertEqual(owner['source'], 'src/F_6B1A.C')
        self.assertEqual(owner['end'] - owner['start'], 48)
        self.assertEqual(recipe['conversions'][0]['id'], 'F_6B1A')
        evidence = read_json(ROOT / 'docs/matching-wave112-evidence.json')
        self.assertEqual(evidence['matched_sha256'], owner['expected_sha256'])
        self.assertEqual(len(evidence['fixups']), 2)
        self.assertEqual(evidence['load_relocations'], [])


if __name__ == '__main__':
    unittest.main()
