import unittest
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from reconstruct import read_json


class SymbolicAsmWave114Tests(unittest.TestCase):
    def test_f4e9f_is_symbolic_asm_owned_with_complete_fixups(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave114.json')
        owner = next(r for r in manifest['regions'] if r['id'] == 'F_4E9F')
        self.assertEqual(owner['kind'], 'MATCHING_ASM')
        self.assertEqual(owner['source'], 'asm/SPRITES.ASM')
        self.assertEqual(owner['end'] - owner['start'], 76)
        self.assertEqual(recipe['format'], 'empires-symbolic-asm-promotion-v1')
        self.assertEqual(recipe['conversions'][0]['id'], 'F_4E9F')
        self.assertNotIn('\ndb ', (ROOT / owner['source']).read_text().lower())
        evidence = read_json(ROOT / 'docs/matching-wave114-evidence.json')
        self.assertEqual(evidence['matched_sha256'], owner['expected_sha256'])
        self.assertEqual(len(evidence['fixups']), 5)
        self.assertEqual(evidence['load_relocations'], [])


if __name__ == '__main__':
    unittest.main()
