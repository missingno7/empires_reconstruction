import unittest
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from reconstruct import read_json


class MatchingCWave128Tests(unittest.TestCase):
    def test_f60a9_is_c_owned_with_loader_relocation(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave128.json')
        owner = next(r for r in manifest['regions'] if r['id'] == 'F_60A9')
        # F_60A9 was reverted to symbolic ASM after the refactor grouped/reviewed
        # asm-origin candidates; see docs/current/asm-origin-review.json. The
        # wave128 evidence below still matches the unchanged original bytes.
        self.assertEqual(owner['kind'], 'MATCHING_ASM')
        self.assertEqual(owner['source'], 'asm/SPRDRAW.ASM')
        self.assertEqual(owner['end'] - owner['start'], 216)
        self.assertEqual(recipe['conversions'][0]['id'], 'F_60A9')
        evidence = read_json(ROOT / 'docs/matching-wave128-evidence.json')
        self.assertEqual(evidence['matched_sha256'], owner['expected_sha256'])
        self.assertEqual(len(evidence['fixups']), 11)
        self.assertEqual(evidence['load_relocations'], [24867])


if __name__ == '__main__':
    unittest.main()
