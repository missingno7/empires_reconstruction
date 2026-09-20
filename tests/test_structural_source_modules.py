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

    def test_keyboard_module_has_contiguous_manifest_ownership(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        modules = structural_source_modules(ROOT, manifest)
        module = next(item for item in modules if item['id'] == 'M_6B1A_6B4A')
        self.assertEqual(module['members'], ['F_6B1A', 'F_6B4A'])
        self.assertEqual(module['end'] - module['start'], 76)

    def test_sound_control_module_has_contiguous_manifest_ownership(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        modules = structural_source_modules(ROOT, manifest)
        module = next(item for item in modules if item['id'] == 'M_C1A0_C232')
        self.assertEqual(module['members'], ['F_C1A0', 'F_C1F7', 'F_C232'])
        self.assertEqual(module['end'] - module['start'], 221)

    def test_command_dispatch_module_has_contiguous_manifest_ownership(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        modules = structural_source_modules(ROOT, manifest)
        module = next(item for item in modules if item['id'] == 'M_C9A4_CA91')
        self.assertEqual(module['members'], [
            'F_C9A4', 'F_CA03', 'F_CA35', 'F_CA51', 'F_CA83', 'F_CA91',
        ])
        self.assertEqual(module['end'] - module['start'], 247)

    def test_command_control_module_has_contiguous_manifest_ownership(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        modules = structural_source_modules(ROOT, manifest)
        module = next(item for item in modules if item['id'] == 'M_C2EA_C501')
        self.assertEqual(module['members'], [
            'F_C2EA', 'F_C359', 'F_C3DB', 'F_C440', 'F_C501',
        ])
        self.assertEqual(module['end'] - module['start'], 607)

    def test_latest_structural_link_stages_one_untouched_decoder_object(self):
        path = ROOT / 'build/tlink-structural-report.json'
        if not path.exists():
            self.skipTest('structural TLINK experiment has not run')
        report = read_json(path)
        expected = {
            'M_6D86_6DCC': 377,
            'M_6B1A_6B4A': 76,
            'M_C1A0_C232': 221,
            'M_C9A4_CA91': 247,
            'M_C2EA_C501': 607,
        }
        staged = {item['owner']: item for item in report['relocatable_scaffold']
                  if item.get('owner') in expected}
        self.assertEqual(set(staged), set(expected))
        for owner, length in expected.items():
            self.assertEqual(staged[owner]['owned_text_length'], length)
            self.assertNotIn('transforms', staged[owner])
        self.assertEqual({item['id'] for item in report['structural_source_modules']}, set(expected))


if __name__ == '__main__':
    unittest.main()
