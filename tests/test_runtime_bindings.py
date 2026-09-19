from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from omf import OmfReader
from reconstruct import read_json
from verify_bss_boundary import verify


class RuntimeBindingTests(unittest.TestCase):
    def test_named_secondary_library_publics_survive_staging(self):
        path = ROOT / 'build/tlink-structural-report.json'
        if not path.exists():
            self.skipTest('local TLINK probe has not run')
        report = read_json(path)
        work = Path(report['byte_comparison']['candidate']).parent
        for owner, symbol in [('F_0281', '_farmalloc'), ('F_2119', '_memmove'),
                              ('F_695E', '_setvect'), ('F_CDDD', '_longjmp')]:
            entry = next(s for s in report['relocatable_scaffold'] if s.get('owner') == owner)
            module = OmfReader().read((work / entry['object']).read_bytes())
            self.assertIn(symbol, module.externals)

    def test_bss_boundary_is_corroborated_by_independent_modules(self):
        result = verify()
        self.assertEqual(result['game_reserve_bytes'], 37250)
        self.assertEqual(result['runtime_bss_bytes'], 68)
        self.assertEqual({w['owner'] for w in result['witnesses']},
                         {'LIB_EXIT', 'LIB_ATEXIT', 'LIB_HARDERR'})
        self.assertFalse(result['internal_game_bss_partition_recovered'])


if __name__ == '__main__':
    unittest.main()
