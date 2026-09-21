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
        # F_4EEB was reverted to symbolic ASM after the refactor grouped/reviewed
        # asm-origin candidates; see docs/current/asm-origin-review.json. The
        # wave116 evidence below still matches the unchanged original bytes.
        self.assertEqual(owner['kind'], 'MATCHING_ASM')
        self.assertEqual(owner['source'], 'asm/F_4EEB.ASM')
        self.assertEqual(owner['end'] - owner['start'], 120)
        self.assertEqual(recipe['conversions'][0]['id'], 'F_4EEB')
        evidence = read_json(ROOT / 'docs/matching-wave116-evidence.json')
        self.assertEqual(evidence['matched_sha256'], owner['expected_sha256'])
        self.assertEqual(len(evidence['fixups']), 6)
        self.assertEqual(evidence['load_relocations'], [])


if __name__ == '__main__':
    unittest.main()
