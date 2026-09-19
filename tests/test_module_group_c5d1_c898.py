"""Fresh shared-compilation proof around the symbolic F_C755 boundary."""
from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))

from probe_module_group import probe


class C5D1C898ModuleGroupTests(unittest.TestCase):
    def test_c_prefix_shares_one_exact_object(self):
        report = probe(ROOT / 'recipes/modules/C_C5D1_C706.json')
        self.assertEqual(report['status'], 'EQUAL')
        self.assertEqual((report['source_units_combined'], report['text_bytes'],
                          report['fixups_checked']), (4, 388, 0))
        self.assertFalse(report['historical_module_proven'])

    def test_c_suffix_shares_one_exact_object(self):
        report = probe(ROOT / 'recipes/modules/C_C77A_C898.json')
        self.assertEqual(report['status'], 'EQUAL')
        self.assertEqual((report['source_units_combined'], report['text_bytes'],
                          report['fixups_checked']), (5, 346, 13))
        self.assertFalse(report['historical_module_proven'])


if __name__ == '__main__':
    unittest.main()
