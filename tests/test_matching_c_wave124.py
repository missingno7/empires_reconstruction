import unittest
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from reconstruct import read_json


class MatchingCWave124Tests(unittest.TestCase):
    def test_fc1a0_is_c_owned_with_fourteen_fixups(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave124.json')
        owner = next(r for r in manifest['regions'] if r['id'] == 'F_C1A0')
        # F_C1A0 was reverted to symbolic ASM after the refactor grouped/reviewed
        # asm-origin candidates; see docs/current/asm-origin-review.json. It now
        # lives inside the single hand-written asm/SOUND.ASM module
        # (M_C1A0_CB48), not the standalone recovery/asm/F_C1A0.ASM. The
        # wave124 evidence below still matches the unchanged original bytes.
        self.assertEqual(owner['kind'], 'MATCHING_ASM')
        self.assertEqual(owner['source'], 'asm/SOUND.ASM')
        self.assertEqual(owner['end'] - owner['start'], 87)
        self.assertEqual(recipe['conversions'][0]['id'], 'F_C1A0')
        evidence = read_json(ROOT / 'docs/matching-wave124-evidence.json')
        self.assertEqual(evidence['matched_sha256'], owner['expected_sha256'])
        self.assertEqual(len(evidence['fixups']), 14)
        self.assertEqual(evidence['load_relocations'], [])


if __name__ == '__main__':
    unittest.main()
