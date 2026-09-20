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
        module = next(item for item in modules if item['id'] == 'M_C27D_C567')
        self.assertEqual(module['members'], [
            'F_C27D', 'F_C2EA', 'F_C359', 'F_C3DB', 'F_C440', 'F_C501', 'F_C549', 'F_C567',
        ])
        self.assertEqual(module['end'] - module['start'], 797)

    def test_menu_control_module_has_contiguous_manifest_ownership(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        modules = structural_source_modules(ROOT, manifest)
        module = next(item for item in modules if item['id'] == 'M_CB5C_CD23')
        self.assertEqual(module['members'], [
            'F_CB5C', 'F_CBBB', 'F_CC17', 'F_CC6B', 'F_CD23',
        ])
        self.assertEqual(module['end'] - module['start'], 641)

    def test_state_table_module_has_contiguous_manifest_ownership(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        modules = structural_source_modules(ROOT, manifest)
        module = next(item for item in modules if item['id'] == 'M_DAD7_DB35')
        self.assertEqual(module['members'], ['F_DAD7', 'F_DB17', 'F_DB35'])
        self.assertEqual(module['end'] - module['start'], 137)

    def test_packed_record_module_has_contiguous_manifest_ownership(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        modules = structural_source_modules(ROOT, manifest)
        module = next(item for item in modules if item['id'] == 'M_D818_D825')
        self.assertEqual(module['members'], ['F_D818', 'F_D825'])
        self.assertEqual(module['end'] - module['start'], 71)

    def test_control_setter_module_has_contiguous_manifest_ownership(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        modules = structural_source_modules(ROOT, manifest)
        module = next(item for item in modules if item['id'] == 'M_C5A8_C5C6')
        self.assertEqual(module['members'], ['F_C5A8', 'F_C5B3', 'F_C5C6'])
        self.assertEqual(module['end'] - module['start'], 41)

    def test_far_record_decoder_module_has_contiguous_manifest_ownership(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        modules = structural_source_modules(ROOT, manifest)
        module = next(item for item in modules if item['id'] == 'M_D386_D3CF')
        self.assertEqual(module['members'], ['F_D386', 'F_D3CF'])
        self.assertEqual(module['end'] - module['start'], 84)

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
            'M_C27D_C567': 688,
            'M_CB5C_CD23': 641,
            'M_DAD7_DB35': 137,
            'M_D818_D825': 71,
            'M_C5A8_C5C6': 41,
            'M_D386_D3CF': 84,
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
