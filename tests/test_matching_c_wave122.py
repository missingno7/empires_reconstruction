import unittest
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from reconstruct import read_json


class MatchingCWave122Tests(unittest.TestCase):
    def test_fd85f_is_c_owned_with_two_fixups(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave122.json')
        owner = next(r for r in manifest['regions'] if r['id'] == 'F_D85F')
        self.assertEqual(owner['kind'], 'MATCHING_C')
        self.assertEqual(owner['source'], 'src/F_D85F.C')
        self.assertEqual(owner['end'] - owner['start'], 59)
        self.assertEqual(recipe['conversions'][0]['id'], 'F_D85F')
        evidence = read_json(ROOT / 'docs/matching-wave122-evidence.json')
        self.assertEqual(evidence['matched_sha256'], owner['expected_sha256'])
        self.assertEqual(len(evidence['fixups']), 2)
        self.assertEqual(evidence['load_relocations'], [])


if __name__ == '__main__':
    unittest.main()
