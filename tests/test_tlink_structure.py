import unittest
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from reconstruct import read_json


class TlinkStructureTests(unittest.TestCase):
    def test_latest_probe_records_natural_prefix_and_first_divergence(self):
        path = ROOT / 'build/tlink-structural-report.json'
        if not path.exists():
            self.skipTest('local TLINK structural probe has not been run')
        report = read_json(path)
        self.assertEqual(report['status'], 'MAP_AVAILABLE')
        comparison = report['code_comparison']
        self.assertGreaterEqual(comparison['actual_code_row_count'], 340)
        divergence = comparison['first_divergence']
        self.assertEqual(divergence['owner'], 'F_F9BE')
        self.assertNotEqual(divergence['expected_start'], divergence['actual_start'])
        self.assertTrue(any(item['kind'] == 'alignment_padding'
                            for item in report['relocatable_scaffold']))


if __name__ == '__main__':
    unittest.main()
