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

    def test_display_adapter_module_has_contiguous_manifest_ownership(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        modules = structural_source_modules(ROOT, manifest)
        module = next(item for item in modules if item['id'] == 'M_50D2_53BF')
        self.assertEqual(module['members'], ['F_50D2', 'F_53BF'])
        self.assertEqual(module['end'] - module['start'], 312)

    def test_paired_display_module_has_contiguous_manifest_ownership(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        modules = structural_source_modules(ROOT, manifest)
        module = next(item for item in modules if item['id'] == 'M_988F_98CB')
        self.assertEqual(module['members'], ['F_988F', 'F_98CB'])
        self.assertEqual(module['end'] - module['start'], 121)

    def test_sound_command_module_has_contiguous_manifest_ownership(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        modules = structural_source_modules(ROOT, manifest)
        module = next(item for item in modules if item['id'] == 'M_C5D1_C706')
        self.assertEqual(module['members'], ['F_C5D1', 'F_C678', 'F_C6B9', 'F_C706'])
        self.assertEqual(module['end'] - module['start'], 388)

    def test_sound_backend_module_has_contiguous_manifest_ownership(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        modules = structural_source_modules(ROOT, manifest)
        module = next(item for item in modules if item['id'] == 'M_C77A_C898')
        self.assertEqual(module['members'], ['F_C77A', 'F_C7CB', 'F_C834', 'F_C877', 'F_C898'])
        self.assertEqual(module['end'] - module['start'], 346)

    def test_record_renderer_module_has_contiguous_manifest_ownership(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        modules = structural_source_modules(ROOT, manifest)
        module = next(item for item in modules if item['id'] == 'M_D61C_D79C')
        self.assertEqual(module['members'], ['F_D61C', 'F_D79C'])
        self.assertEqual(module['end'] - module['start'], 507)

    def test_production_plan_stages_untouched_structural_modules(self):
        from omf import OmfReader
        path = ROOT / 'build/exe-build-report.json'
        if not path.exists():
            self.skipTest('canonical production build has not run')
        report = read_json(path)
        self.assertEqual(report['link_invocations'], 1)
        plan = read_json(ROOT / 'layout/production-plan.json')
        structural = read_json(ROOT / 'layout/structural-source-modules.json')['modules']
        staged = {m['id']:m for m in plan['modules']}
        work = Path(report['session'])
        for module in structural:
            item = staged[module['id']]
            original_path = report.get('compiled_objects', {}).get(module['id'], 'WORK/' + item['object'])
            original = (work / 'compile' / original_path).read_bytes()
            linked = (work / 'WORK' / item['object']).read_bytes()
            self.assertEqual(original, linked, module['id'])
            self.assertEqual(OmfReader().read(linked).segment_length('_TEXT'), module['end'] - module['start'])


if __name__ == '__main__':
    unittest.main()
