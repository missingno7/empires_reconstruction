from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from probe_data_interleaving import interleave
from reconstruct import read_json


class InterleavingTests(unittest.TestCase):
    def test_move_preserves_code_and_data_orders(self):
        objects = ['start', 'a.obj', 'b.obj', 'd1.obj', 'd2.obj', 'd3.obj', 'bss.obj']
        result = interleave(objects, [{'data_through': 'D2', 'after_code': 'A', 'before_next_relocating_code': 'B'}],
                            [('D1', 'd1.obj'), ('D2', 'd2.obj'), ('D3', 'd3.obj')],
                            {'A': 'a.obj', 'B': 'b.obj'})
        self.assertEqual(result, ['start', 'a.obj', 'd1.obj', 'd2.obj', 'b.obj', 'd3.obj', 'bss.obj'])
        with self.assertRaisesRegex(ValueError, 'insertion interval'):
            interleave(objects, [{'data_through': 'D1', 'after_code': 'B', 'before_next_relocating_code': 'A'}],
                       [('D1', 'd1.obj')], {'A': 'a.obj', 'B': 'b.obj'})

    def test_real_interleaving_preserves_image_and_extends_matching_prefix(self):
        path = ROOT / 'build/data-interleaving-report.json'
        if not path.exists():
            self.skipTest('local interleaving experiment has not run')
        report = read_json(path)
        self.assertEqual(report['status'], 'LAYOUT_PRESERVED')
        self.assertTrue(report['byte_comparison']['load_image']['equal'])
        self.assertTrue(report['byte_comparison']['mz']['relocation_pairs_equal'])
        self.assertEqual(report['matching_relocation_prefix_entries'], 36)
        self.assertFalse(report['historical_module_proven'])


if __name__ == '__main__':
    unittest.main()
