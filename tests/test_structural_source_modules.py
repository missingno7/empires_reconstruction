from pathlib import Path
import sys
import unittest


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))

from probe_tlink_layout import structural_source_modules
from reconstruct import read_json


class StructuralSourceModuleTests(unittest.TestCase):
    def test_decoder_module_has_contiguous_manifest_ownership(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        modules = structural_source_modules(ROOT, manifest)
        module = next(item for item in modules if item['id'] == 'M_6D86_6DCC')
        self.assertEqual(module['members'], ['F_6D86', 'PAD_006FC5', 'F_6DCC'])
        self.assertEqual(module['end'] - module['start'], 377)

    def test_latest_structural_link_stages_one_untouched_decoder_object(self):
        path = ROOT / 'build/tlink-structural-report.json'
        if not path.exists():
            self.skipTest('structural TLINK experiment has not run')
        report = read_json(path)
        entry = next(item for item in report['relocatable_scaffold']
                     if item.get('owner') == 'M_6D86_6DCC')
        self.assertEqual(entry['owned_text_length'], 377)
        self.assertNotIn('transforms', entry)
        self.assertIn('M_6D86_6DCC',
                      [item['id'] for item in report['structural_source_modules']])


if __name__ == '__main__':
    unittest.main()
