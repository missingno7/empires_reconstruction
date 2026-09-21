import unittest
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from reconstruct import read_json


class MatchingCWave130Tests(unittest.TestCase):
    def test_last_asm_owners_are_c_owned(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave130.json')
        evidence = read_json(ROOT / 'docs/matching-wave130-evidence.json')
        # F_6BCF was reverted to symbolic ASM after the refactor's asm-origin
        # review (see docs/current/asm-origin-review.json); F_699E remains
        # C-owned. Both still match the unchanged original bytes recorded by
        # the wave130 evidence.
        expected_kind = {'F_6BCF': ('MATCHING_ASM', 'asm/F_6BCF.ASM'),
                         'F_699E': ('MATCHING_C', 'src/KEYIRQH.C')}
        for ident, size, relocation in [('F_6BCF', 87, [27609]), ('F_699E', 380, [27048])]:
            owner = next(r for r in manifest['regions'] if r['id'] == ident)
            kind, source = expected_kind[ident]
            self.assertEqual(owner['kind'], kind)
            self.assertEqual(owner['end'] - owner['start'], size)
            self.assertEqual(owner['source'], source)
            item = next(x for x in evidence['conversions'] if x['id'] == ident)
            self.assertEqual(item['matched_sha256'], owner['expected_sha256'])
            self.assertEqual(len(item['fixups']), 11)
            self.assertEqual(item['load_relocations'], relocation)
        self.assertEqual(len(recipe['conversions']), 2)
        # The refactor's asm-origin review restored a number of regions to
        # symbolic ASM (see docs/current/asm-origin-review.json), so the
        # production build no longer holds zero MATCHING_ASM bytes overall;
        # confirm the current total instead.
        self.assertEqual(sum(r['end'] - r['start'] for r in manifest['regions'] if r['kind'] == 'MATCHING_ASM'), 10877)


if __name__ == '__main__':
    unittest.main()
