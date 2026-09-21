"""Retained compiler DATA must respect natural code and DATA ordering."""
import sys,unittest
from pathlib import Path
from unittest.mock import patch
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import production_plan

class NativeDataPlanTests(unittest.TestCase):
    def test_native_objects_replace_only_duplicate_data_inputs(self):
        p=production_plan.generate()
        retained=[d for d in p['data'] if d.get('retain_in_code_object')]
        self.assertEqual({d['code_owner'] for d in retained},{'F_A28D','F_D5BA','F_9D8E','F_75F3','F_7695','F_778B'})
        for d in retained:
            m=next(m for m in p['modules'] if d['code_owner'] in m['members'])
            self.assertNotIn(d['object'],p['object_order'])
            self.assertEqual(p['object_order'].count(m['object']),1)
        code=[m['object'] for m in p['modules'] if not m['build'].get('linker_library_module')]
        self.assertEqual([x for x in p['object_order'] if x in set(code)],code)

    def test_padding_is_owned_by_source_segment_alignment(self):
        p=production_plan.generate()
        self.assertEqual(p['padding'], [])
        self.assertEqual({a['before_module'] for a in p['linker_alignment']},
                         {'M_4AA8_4EEB','M_6D86_6F4B','M_D61C_D79C','M_D818_D825'})
        self.assertFalse(any(name.startswith('P') for name in p['object_order']))

    def test_invalid_alignment_ownership_is_rejected(self):
        original=production_plan.read_json
        def changed(path):
            obj=original(path)
            if path.name=='manifest.json':
                next(o for o in obj['regions'] if o['id']=='PAD_004CA7')['build']['linker_alignment_before']='F_01BC'
            return obj
        with patch.object(production_plan,'read_json',changed):
            with self.assertRaisesRegex(ValueError,'Invalid natural word-alignment ownership'):
                production_plan.generate()

    def test_incompatible_native_data_order_is_rejected(self):
        original=production_plan.read_json
        def changed(path):
            obj=original(path)
            if path.name=='canonical-link-plan.json':
                next(c for c in obj['data_modules'] if c['id']=='HELP_MENU_DATA_SUFFIX')['after_code']='F_7BFC'
            return obj
        with patch.object(production_plan,'read_json',changed):
            with self.assertRaisesRegex(ValueError,'DATA.*order'):
                production_plan.generate()

if __name__=='__main__':unittest.main()
