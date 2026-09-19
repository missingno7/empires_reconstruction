from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from discover_relocation_groups import candidates
from reconstruct import read_json


class RelocationGroupTests(unittest.TestCase):
    def test_interval_includes_owners_without_relocations(self):
        owners = [{'id': name, 'start': 512 + 10*i, 'end': 522 + 10*i,
                   'kind': 'MATCHING_C', 'build': {'flags_append': ''}}
                  for i, name in enumerate(['A', 'B', 'C'])]
        groups = candidates({'regions': owners}, [{'load_offset': 24}, {'load_offset': 3}])
        self.assertEqual(groups[0]['owners'], ['A', 'B', 'C'])

    def test_shared_groups_preserve_image_and_fix_their_relocation_order(self):
        for candidate, relocations in [('C_75F3_7856', 3), ('RELOC_F_AD25_F_ADCF', 6)]:
            path = ROOT / 'build' / ('shared-source-data-link-report_' + candidate + '.json')
            if not path.exists():
                self.skipTest('local shared-source link has not run')
            report = read_json(path)
            self.assertTrue(report['source_data_mode'])
            self.assertTrue(report['byte_comparison']['load_image']['equal'])
            self.assertTrue(report['group_relocation_order']['equal'])
            self.assertEqual(len(report['group_relocation_order']['actual']), relocations)
            self.assertFalse(report['historical_module_proven'])

    def test_arithmetic_group_adapter_preserves_image_and_fixupp_order(self):
        path = ROOT / 'build/shared-source-data-link-report_RELOC_F_DDD9_F_DF98.json'
        if not path.exists():
            self.skipTest('local arithmetic shared-source link has not run')
        report = read_json(path)
        self.assertTrue(report['source_data_mode'])
        self.assertTrue(report['byte_comparison']['load_image']['equal'])
        self.assertEqual(len(report['group_relocation_order']['actual']), 10)
        self.assertTrue(report['group_relocation_order']['equal'])
        self.assertEqual(report['fixupp_order_adapter'],
                         {'segment': '_TEXT', 'order': 'descending'})
        self.assertFalse(report['historical_module_proven'])


if __name__ == '__main__':
    unittest.main()
