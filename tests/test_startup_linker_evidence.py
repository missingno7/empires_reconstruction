import unittest
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from reconstruct import read_json


class StartupLinkerEvidenceTests(unittest.TestCase):
    def test_startup_object_evidence_is_complete(self):
        path = ROOT / 'docs/startup-linker-evidence.json'
        if not path.exists():
            self.skipTest('local startup-object audit has not been run')
        evidence = read_json(path)
        self.assertEqual(evidence['status'], 'EQUAL')
        self.assertEqual(evidence['declared_text_bytes'], 0x1BC)
        self.assertEqual(evidence['non_fixup_mismatches'], [])
        self.assertEqual(evidence['segment_definition']['alignment'], 'byte')
        self.assertEqual(evidence['segment_definition']['combine'], 'public')


if __name__ == '__main__':
    unittest.main()
