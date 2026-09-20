from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from analyze_runtime_block import analyze


class RuntimeBlockCfgTests(unittest.TestCase):
    def test_dispatch_veneers_have_stable_bounded_roots(self):
        report = analyze((ROOT / 'build/regions/RUNTIME_BLOCK.bin').read_bytes(),
                         ROOT / 'src/RUNTIME_BLOCK.C')
        self.assertEqual(report['bytes'], 6571)
        self.assertEqual(report['root_count'], 20)
        self.assertEqual(report['roots'][0]['aliases'], ['_f039c'])
        self.assertEqual(report['roots'][1]['aliases'], ['_f039f', '_box'])
        self.assertEqual(report['roots'][-1]['target_offset'], 6511)
        self.assertTrue(all(60 <= item['target_offset'] < 6571 for item in report['roots']))


if __name__ == '__main__':
    unittest.main()
