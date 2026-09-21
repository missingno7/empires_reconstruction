import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from reconstruct import read_json


class MatchingCWave113Tests(unittest.TestCase):
    def test_f4aa8_is_c_owned_with_one_fixup(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave113.json')
        owner = next(r for r in manifest['regions'] if r['id'] == 'F_4AA8')
        # F_4AA8 was reverted to symbolic ASM after the refactor grouped/reviewed
        # asm-origin candidates; see docs/current/asm-origin-review.json. The
        # wave113 evidence below still matches the unchanged original bytes.
        self.assertEqual(owner['kind'], 'MATCHING_ASM')
        self.assertEqual(owner['source'], 'asm/SPRITES.ASM')
        self.assertEqual(owner['end'] - owner['start'], 100)
        self.assertEqual(recipe['conversions'][0]['id'], 'F_4AA8')
        evidence = read_json(ROOT / 'docs/matching-wave113-evidence.json')
        self.assertEqual(evidence['matched_sha256'], owner['expected_sha256'])
        self.assertEqual(len(evidence['fixups']), 1)
        self.assertEqual(evidence['load_relocations'], [])


if __name__ == '__main__':
    unittest.main()
