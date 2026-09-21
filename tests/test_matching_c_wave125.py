import unittest
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from reconstruct import read_json


class MatchingCWave125Tests(unittest.TestCase):
    def test_f6d86_is_c_owned_without_fixups(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave125.json')
        owner = next(r for r in manifest['regions'] if r['id'] == 'F_6D86')
        # F_6D86 was reverted to symbolic ASM after the refactor grouped/reviewed
        # asm-origin candidates; see docs/current/asm-origin-review.json. The
        # wave125 evidence below still matches the unchanged original bytes.
        self.assertEqual(owner['kind'], 'MATCHING_ASM')
        self.assertEqual(owner['source'], 'recovery/asm/F_6D86.ASM')
        self.assertEqual(owner['end'] - owner['start'], 63)
        self.assertEqual(recipe['conversions'][0]['id'], 'F_6D86')
        evidence = read_json(ROOT / 'docs/matching-wave125-evidence.json')
        self.assertEqual(evidence['matched_sha256'], owner['expected_sha256'])
        self.assertEqual(evidence['fixups'], [])
        self.assertEqual(evidence['load_relocations'], [])


if __name__ == '__main__':
    unittest.main()
