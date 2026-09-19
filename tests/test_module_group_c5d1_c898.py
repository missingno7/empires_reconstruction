"""Fresh shared-compilation proof for the C5D1..C898 source module candidate."""
from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))

from probe_module_group import probe


class C5D1C898ModuleGroupTests(unittest.TestCase):
    def test_contiguous_sources_share_one_exact_object(self):
        report = probe(ROOT / 'recipes/modules/C_C5D1_C898.json')
        self.assertEqual(report['status'], 'EQUAL')
        self.assertEqual((report['source_units_combined'], report['text_bytes'],
                          report['fixups_checked']), (10, 771, 11))
        self.assertFalse(report['historical_module_proven'])


if __name__ == '__main__':
    unittest.main()
