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
        # review and recovered again as a Turbo C interrupt function with a
        # bare pushf/popf inline pair; its unit (src/TIMERIRQ.C) was later
        # folded into the src/TIMER.C translation-unit merge (module
        # C_6B7A_6C87). F_699E remains C-owned; its unit (src/KEYIRQH.C) was
        # folded into the src/KEYBOARD.C merge (module C_6990_6B74). See
        # docs/current/asm-provenance.json. Both still match the unchanged
        # original bytes recorded by the wave130 evidence.
        expected_kind = {'F_6BCF': ('MATCHING_C', 'src/TIMER.C'),
                         'F_699E': ('MATCHING_C', 'src/KEYBOARD.C')}
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
        # confirm the current total instead. It grew from the prior 10790 with
        # the sound-driver re-closure: F_C27D and F_C834 (formerly C
        # candidates) are now proven ASM members of asm/SOUND.ASM
        # (M_C1A0_CB48; see docs/current/asm-provenance.json).
        self.assertEqual(sum(r['end'] - r['start'] for r in manifest['regions'] if r['kind'] == 'MATCHING_ASM'), 11442)


if __name__ == '__main__':
    unittest.main()
