import copy
from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from mz import MZ
from reconstruct import component_binding, owned_library_modules, read_json
from verify_runtime_data_tail import verify


class RuntimeDataTailTests(unittest.TestCase):
    def test_actual_tlink_tail(self):
        if not (ROOT / 'build/tlink-structural-report.json').exists():
            self.skipTest('local TLINK probe has not run')
        report = verify()
        self.assertEqual(report['data_bytes'], 420)
        self.assertEqual(len(report['modules']), 10)
        self.assertEqual(sum(m['fixups_checked'] for m in report['modules']), 3)

    def test_exit_segment_base_differs_from_exit_public(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        modules = owned_library_modules(manifest['regions'], ROOT / 'toolchain',
                                        read_json(ROOT / 'layout/toolchain.json'))
        owner = next(o for o in manifest['regions'] if o['id'] == 'LIB_EXIT_DATA')
        binding = owner['build']['module_segments']['_TEXT']
        mz = MZ.parse((ROOT / 'assets/AEPROG.EXE').read_bytes())
        def resolve(value):
            return component_binding(value, manifest['regions'], mz, manifest['frames'], modules)
        base = resolve(binding)['offset']
        public = dict(binding, public='_exit')
        del public['module_segment']
        self.assertEqual(resolve(public)['offset'], base + 1)
        bad = copy.deepcopy(binding)
        bad['module_segment'] = '_DATA'
        with self.assertRaisesRegex(ValueError, 'complete matching ownership'):
            resolve(bad)
        with self.assertRaisesRegex(ValueError, 'complete matching ownership'):
            resolve(dict(binding, public='_exit'))


if __name__ == '__main__':
    unittest.main()
