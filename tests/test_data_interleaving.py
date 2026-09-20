from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from canonical_link_plan import order_data_modules
from reconstruct import read_json


class InterleavingTests(unittest.TestCase):
    def test_canonical_plan_preserves_code_and_data_orders(self):
        objects = ['start', 'a.obj', 'b.obj', 'd1.obj', 'd2.obj', 'd3.obj', 'bss.obj']
        plan = {'format': 'empires-canonical-data-link-plan-v1', 'data_modules': [
            {'id': 'DATA', 'through': 'D2', 'after_code': 'A', 'before_code': 'B'}]}
        result, placements = order_data_modules(objects, [('D1', 'd1.obj'), ('D2', 'd2.obj'), ('D3', 'd3.obj')],
                                                {'A': 'a.obj', 'B': 'b.obj'}, plan)
        self.assertEqual(result, ['start', 'a.obj', 'd1.obj', 'd2.obj', 'b.obj', 'd3.obj', 'bss.obj'])
        self.assertEqual(placements[0]['module'], 'DATA')
        bad = {'format': 'empires-canonical-data-link-plan-v1', 'data_modules': [
            {'id': 'BAD', 'through': 'D1', 'after_code': 'B', 'before_code': 'A'}]}
        with self.assertRaisesRegex(ValueError, 'insertion interval'):
            order_data_modules(objects, [('D1', 'd1.obj')], {'A': 'a.obj', 'B': 'b.obj'}, bad)

    def test_canonical_plan_is_recorded_by_source_data_link(self):
        path = ROOT / 'build/source-data-link-report.json'
        if not path.exists():
            self.skipTest('local source-DATA link has not run')
        report = read_json(path)
        self.assertEqual(report['status'], 'LINKED')
        self.assertTrue(report['byte_comparison']['load_image']['equal'])
        self.assertTrue(report['byte_comparison']['mz']['relocation_pairs_equal'])
        plan = report['canonical_link_plan']
        self.assertEqual(plan['status'], 'COMPATIBLE_RECONSTRUCTED_MODULES')
        self.assertEqual(len(plan['placements']), 4)
        self.assertFalse(plan['historical_translation_units_proven'])


if __name__ == '__main__':
    unittest.main()
