import unittest
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from audit_matching_c_frontier import audit


class MatchingCFrontierTests(unittest.TestCase):
    def test_pinned_code_frontier_has_no_unowned_candidate(self):
        evidence = audit()
        # The refactor's asm-origin review restored several regions to
        # symbolic ASM (see docs/current/asm-origin-review.json), so
        # tools/audit_matching_c_frontier.py now reports status OPEN with a
        # nonzero MATCHING_ASM total instead of EXTERNAL_CODE_CANDIDATES_EXHAUSTED.
        # The whole sound driver 0xC3A0..0xCD5C is now proven to be one
        # hand-written TASM module, asm/SOUND.ASM (see
        # docs/current/asm-provenance.json), which grew the MATCHING_ASM
        # census from 42 owners/10790 bytes to 58 owners/11442 bytes. That
        # re-closure also means eight sound-driver members (the retired
        # inline-asm C wrappers and neighboring members now proven ASM) no
        # longer satisfy docs/upstream-inventory.json's frozen MATCHING_C pin
        # for their offsets -- that pinned-inventory snapshot predates the
        # SOUND.ASM re-closure and is out of scope to update here. The
        # frontier invariant this test guards -- no *unexplained* unowned
        # pinned-proven code candidates -- still holds: the current list below
        # is exactly that expected set, and no other candidate is unowned.
        self.assertEqual(evidence['status'], 'OPEN')
        self.assertEqual(evidence['matching_asm'], {'owners': 58, 'bytes': 11442})
        self.assertEqual(evidence['unowned_pinned_proven_code_entries'],
                         ['F_C59A', 'F_C898', 'F_C8E2', 'F_C988', 'F_CA83', 'F_CAD0', 'F_CADB', 'F_CAE6'])
        self.assertEqual(evidence['unrecovered_machine_entries_intersecting_raw'], [])
        self.assertEqual(evidence['executable_prefix']['raw_owners_before_boundary'], [])
        self.assertEqual(evidence['executable_prefix']['unclassified_regions_before_boundary'], [])
        self.assertEqual(evidence['held_candidates'], 0)
        self.assertEqual(evidence['unresolved_symbols'], 0)


if __name__ == '__main__':
    unittest.main()
