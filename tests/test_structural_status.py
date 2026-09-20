from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from reconstruct import read_json


class StructuralStatusTests(unittest.TestCase):
    def test_checked_in_status_describes_canonical_linked_build(self):
        status = read_json(ROOT / 'docs/structural-status.json')
        self.assertEqual(status['link_status'], 'BUILT')
        self.assertEqual(status['unresolved_count'], 0)
        self.assertEqual(status['synthetic_data_bytes'], 0)
        self.assertEqual(status['synthetic_bss_bytes'], 0)
        self.assertIsNone(status['first_code_placement_divergence'])
        self.assertTrue(status['byte_comparison']['full_file']['equal'])
        self.assertEqual(status['byte_comparison']['candidate_sha256'],
                         '1259348425483d8d97fd8821860b47cfdf58fc8029711eb0ed0e78ab33807a10')
        self.assertTrue(status['byte_comparison']['mz']['relocation_order_equal'])
        self.assertEqual(status['byte_comparison']['mz']['candidate_fields']['e_crlc'], 106)
        self.assertEqual(status['baseline_diagnostic']['synthetic_bss_bytes'], 37254)


if __name__ == '__main__':
    unittest.main()
