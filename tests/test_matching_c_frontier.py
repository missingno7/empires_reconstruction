import unittest
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from audit_matching_c_frontier import audit


class MatchingCFrontierTests(unittest.TestCase):
    def test_pinned_code_frontier_has_no_unowned_candidate(self):
        evidence = audit()
        self.assertEqual(evidence['status'], 'EXTERNAL_CODE_CANDIDATES_EXHAUSTED')
        self.assertEqual(evidence['matching_asm'], {'owners': 0, 'bytes': 0})
        self.assertEqual(evidence['unowned_pinned_proven_code_entries'], [])
        self.assertEqual(evidence['unrecovered_machine_entries_intersecting_raw'], [])
        self.assertEqual(evidence['executable_prefix']['raw_owners_before_boundary'], [])
        self.assertEqual(evidence['executable_prefix']['unclassified_regions_before_boundary'], [])
        self.assertEqual(evidence['held_candidates'], 0)
        self.assertEqual(evidence['unresolved_symbols'], 0)


if __name__ == '__main__':
    unittest.main()
