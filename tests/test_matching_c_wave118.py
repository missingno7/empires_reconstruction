import unittest
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from reconstruct import read_json


class MatchingCWave118Tests(unittest.TestCase):
    def test_fc1f7_is_c_owned_with_thirteen_fixups(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave118.json')
        owner = next(r for r in manifest['regions'] if r['id'] == 'F_C1F7')
        # F_C1F7 was reverted to symbolic ASM after the refactor grouped/reviewed
        # asm-origin candidates; see docs/current/asm-origin-review.json. The
        # wave118 evidence below still matches the unchanged original bytes.
        self.assertEqual(owner['kind'], 'MATCHING_ASM')
        self.assertEqual(owner['source'], 'recovery/asm/F_C1F7.ASM')
        self.assertEqual(owner['end'] - owner['start'], 59)
        self.assertEqual(recipe['conversions'][0]['id'], 'F_C1F7')
        evidence = read_json(ROOT / 'docs/matching-wave118-evidence.json')
        self.assertEqual(evidence['matched_sha256'], owner['expected_sha256'])
        self.assertEqual(len(evidence['fixups']), 13)
        self.assertEqual(evidence['load_relocations'], [])


if __name__ == '__main__':
    unittest.main()
