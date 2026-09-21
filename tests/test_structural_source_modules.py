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
        module = next(item for item in modules if item['id'] == 'M_6D86_6F4B')
        self.assertEqual(module['members'], ['F_6D86', 'PAD_006FC5', 'F_6DCC', 'F_6EFF', 'F_6F4B'])
        self.assertEqual(module['end'] - module['start'], 573)

    # M_6B1A_6B4A was removed from layout/structural-source-modules.json: its
    # members (F_6B1A, F_6B4A) are now inside the src/KEYBOARD.C
    # translation-unit merge (module C_6990_6B74 in
    # layout/production-plan.json; see docs/current/asm-provenance.json)
    # rather than a standalone structural ASM/inline-asm source module.

    # M_C1A0_C232, M_C27D_C567, M_C5A8_C5C6, M_C5D1_C706, M_C77A_C898 and
    # M_C9A4_CA91 were removed from layout/structural-source-modules.json:
    # the whole sound driver 0xC3A0..0xCD5C they used to carve up is now
    # proven to be one hand-written TASM module, asm/SOUND.ASM (see
    # docs/current/asm-provenance.json), replaced below by a single
    # contiguous-ownership test for M_C1A0_CB48.
    def test_sound_backend_module_has_contiguous_manifest_ownership(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        modules = structural_source_modules(ROOT, manifest)
        module = next(item for item in modules if item['id'] == 'M_C1A0_CB48')
        self.assertEqual(len(module['members']), 41)
        self.assertEqual(module['members'][0], 'F_C1A0')
        self.assertEqual(module['members'][-1], 'F_CB48')
        self.assertEqual(module['end'] - module['start'], 2492)

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

    # M_C5A8_C5C6 was removed from layout/structural-source-modules.json for
    # the same reason as the other sound sub-modules above: it is now part of
    # the single hand-written asm/SOUND.ASM module, covered by
    # test_sound_backend_module_has_contiguous_manifest_ownership.

    def test_far_record_decoder_module_has_contiguous_manifest_ownership(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        modules = structural_source_modules(ROOT, manifest)
        module = next(item for item in modules if item['id'] == 'M_D386_D3CF')
        self.assertEqual(module['members'], ['F_D386', 'F_D3CF'])
        self.assertEqual(module['end'] - module['start'], 84)

    # M_50D2_53BF was removed from layout/structural-source-modules.json: its
    # members (F_50D2, F_53BF) are now recovered as exact C and folded into
    # the src/STARTUP.C translation-unit merge (module C_4F63_520A in
    # layout/production-plan.json; see docs/current/asm-provenance.json)
    # rather than a standalone structural ASM source module.

    def test_paired_display_module_has_contiguous_manifest_ownership(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        modules = structural_source_modules(ROOT, manifest)
        module = next(item for item in modules if item['id'] == 'M_988F_98CB')
        self.assertEqual(module['members'], ['F_988F', 'F_98CB'])
        self.assertEqual(module['end'] - module['start'], 121)

    # M_C5D1_C706 was removed from layout/structural-source-modules.json for
    # the same reason as the other sound sub-modules above: it is now part of
    # the single hand-written asm/SOUND.ASM module, covered by
    # test_sound_backend_module_has_contiguous_manifest_ownership.

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
