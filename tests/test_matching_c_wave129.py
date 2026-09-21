import unittest
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from reconstruct import read_json


class MatchingCWave129Tests(unittest.TestCase):
    def test_f6dcc_is_c_owned_without_relocations(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave129.json')
        owner = next(r for r in manifest['regions'] if r['id'] == 'F_6DCC')
        # F_6DCC was consolidated with F_6D86/PAD_006FC5/F_6EFF/F_6F4B into one
        # word-alignment-proven TASM module (asm/DECODE.ASM, id M_6D86_6F4B);
        # it was already compiled as part of that ASM module, never
        # independently as C, so its manifest kind now matches (MATCHING_ASM).
        self.assertEqual(owner['kind'], 'MATCHING_ASM')
        self.assertEqual(owner['source'], 'asm/DECODE.ASM')
        self.assertEqual(owner['end'] - owner['start'], 307)
        self.assertEqual(recipe['conversions'][0]['id'], 'F_6DCC')
        evidence = read_json(ROOT / 'docs/matching-wave129-evidence.json')
        self.assertEqual(evidence['matched_sha256'], owner['expected_sha256'])
        self.assertEqual(evidence['fixups'], [])
        self.assertEqual(evidence['load_relocations'], [])


if __name__ == '__main__':
    unittest.main()
