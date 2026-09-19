from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from reconstruct import read_json


class ExactStructuralLinkTests(unittest.TestCase):
    def test_local_complete_experiment_is_exact_and_still_adapter_backed(self):
        path = ROOT / 'build/exact-structural-link-report.json'
        if not path.exists():
            self.skipTest('local exact structural-link experiment has not run')
        report = read_json(path)
        self.assertEqual(report['status'], 'BYTE_IDENTICAL_STRUCTURAL_EXPERIMENT')
        self.assertTrue(report['byte_comparison']['full_file']['equal'])
        self.assertTrue(report['byte_comparison']['mz']['relocation_order_equal'])
        self.assertEqual(report['byte_comparison']['candidate_sha256'],
                         '1259348425483d8d97fd8821860b47cfdf58fc8029711eb0ed0e78ab33807a10')
        self.assertTrue(report['remaining_adapters'])
        self.assertFalse(report['whole_build_reconstruction_complete'])


if __name__ == '__main__':
    unittest.main()
