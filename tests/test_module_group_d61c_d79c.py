"""Fresh shared-compilation proof for the D61C..D79C decoder pair."""
from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))

from probe_module_group import probe


class D61CD79CModuleGroupTests(unittest.TestCase):
    def test_contiguous_decoder_pair_has_one_exact_object(self):
        report = probe(ROOT / 'recipes/modules/C_D61C_D79C.json')
        self.assertEqual(report['status'], 'EQUAL')
        self.assertEqual((report['source_units_combined'], report['text_bytes'],
                          report['fixups_checked']), (2, 507, 0))
        self.assertFalse(report['historical_module_proven'])


if __name__ == '__main__':
    unittest.main()
