import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from reconstruct import read_json


class SymbolicAsmWave110Tests(unittest.TestCase):
    def test_fd3cf_is_symbolic_asm_owned_with_boundary_proof(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave110.json')
        owner = next(r for r in manifest['regions'] if r['id'] == 'F_D3CF')
        self.assertEqual(owner['kind'], 'MATCHING_ASM')
        self.assertEqual(owner['source'], 'asm/F_D3CF.ASM')
        self.assertEqual(owner['end'] - owner['start'], 11)
        self.assertEqual(recipe['format'], 'empires-symbolic-asm-promotion-v1')
        self.assertEqual(recipe['conversions'][0]['id'], 'F_D3CF')
        self.assertNotIn('\ndb ', (ROOT / owner['source']).read_text().lower())
        evidence = read_json(ROOT / 'docs/matching-wave110-evidence.json')
        self.assertEqual(evidence['matched_sha256'], owner['expected_sha256'])
        self.assertEqual(evidence['fixups'], [])
        self.assertEqual(evidence['load_relocations'], [])


if __name__ == '__main__':
    unittest.main()
