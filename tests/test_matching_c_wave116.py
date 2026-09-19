import unittest
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from reconstruct import read_json


class MatchingCWave116Tests(unittest.TestCase):
    def test_f4eeb_is_c_owned_with_six_fixups(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave116.json')
        owner = next(r for r in manifest['regions'] if r['id'] == 'F_4EEB')
        self.assertEqual(owner['kind'], 'MATCHING_C')
        self.assertEqual(owner['source'], 'src/F_4EEB.C')
        self.assertEqual(owner['end'] - owner['start'], 120)
        self.assertEqual(recipe['conversions'][0]['id'], 'F_4EEB')
        evidence = read_json(ROOT / 'docs/matching-wave116-evidence.json')
        self.assertEqual(evidence['matched_sha256'], owner['expected_sha256'])
        self.assertEqual(len(evidence['fixups']), 6)
        self.assertEqual(evidence['load_relocations'], [])


if __name__ == '__main__':
    unittest.main()
