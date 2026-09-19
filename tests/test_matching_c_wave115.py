import unittest
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from reconstruct import read_json


class MatchingCWave115Tests(unittest.TestCase):
    def test_f1f91_is_c_owned_with_one_fixup(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave115.json')
        owner = next(r for r in manifest['regions'] if r['id'] == 'F_1F91')
        self.assertEqual(owner['kind'], 'MATCHING_C')
        self.assertEqual(owner['source'], 'src/F_1F91.C')
        self.assertEqual(owner['end'] - owner['start'], 126)
        self.assertEqual(recipe['conversions'][0]['id'], 'F_1F91')
        evidence = read_json(ROOT / 'docs/matching-wave115-evidence.json')
        self.assertEqual(evidence['matched_sha256'], owner['expected_sha256'])
        self.assertEqual(len(evidence['fixups']), 1)
        self.assertEqual(evidence['load_relocations'], [])


if __name__ == '__main__':
    unittest.main()
