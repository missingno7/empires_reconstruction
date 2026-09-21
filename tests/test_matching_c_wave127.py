import unittest
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from reconstruct import read_json


class MatchingCWave127Tests(unittest.TestCase):
    def test_f50d2_is_c_owned_with_thirteen_fixups(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave127.json')
        owner = next(r for r in manifest['regions'] if r['id'] == 'F_50D2')
        # F_50D2 was reverted to symbolic ASM after the refactor grouped/reviewed
        # asm-origin candidates; see docs/current/asm-origin-review.json. The
        # wave127 evidence below still matches the unchanged original bytes.
        self.assertEqual(owner['kind'], 'MATCHING_ASM')
        self.assertEqual(owner['source'], 'asm/F_50D2.ASM')
        self.assertEqual(owner['end'] - owner['start'], 237)
        self.assertEqual(recipe['conversions'][0]['id'], 'F_50D2')
        evidence = read_json(ROOT / 'docs/matching-wave127-evidence.json')
        self.assertEqual(evidence['matched_sha256'], owner['expected_sha256'])
        self.assertEqual(len(evidence['fixups']), 13)
        self.assertEqual(evidence['load_relocations'], [])


if __name__ == '__main__':
    unittest.main()
