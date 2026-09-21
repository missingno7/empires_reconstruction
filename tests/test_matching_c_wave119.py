import unittest
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from reconstruct import read_json


class MatchingCWave119Tests(unittest.TestCase):
    def test_fc27d_is_c_owned_with_eighteen_fixups(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave119.json')
        owner = next(r for r in manifest['regions'] if r['id'] == 'F_C27D')
        # F_C27D's C candidate (recovery/src/F_C27D.C) is overturned: the whole
        # sound driver 0xC3A0..0xCD5C is now proven to be one hand-written TASM
        # module, asm/SOUND.ASM (M_C1A0_CB48); see
        # docs/current/asm-provenance.json for why Turbo C cannot reproduce it
        # (frames that never save SI/DI). The wave119 evidence below still
        # matches the unchanged original bytes.
        self.assertEqual(owner['kind'], 'MATCHING_ASM')
        self.assertEqual(owner['source'], 'asm/SOUND.ASM')
        self.assertEqual(owner['end'] - owner['start'], 109)
        self.assertEqual(recipe['conversions'][0]['id'], 'F_C27D')
        evidence = read_json(ROOT / 'docs/matching-wave119-evidence.json')
        self.assertEqual(evidence['matched_sha256'], owner['expected_sha256'])
        self.assertEqual(len(evidence['fixups']), 18)
        self.assertEqual(evidence['load_relocations'], [])


if __name__ == '__main__':
    unittest.main()
