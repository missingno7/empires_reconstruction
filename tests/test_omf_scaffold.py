import unittest
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from omf import OmfReader
from omf_scaffold import (make_text_padding, remove_public, trim_text_contribution,
                          make_dgroup_scaffold, order_explicit_fixupp_subrecords,
                          ensure_turbo_c_dgroup, _records)
from pointer_records import records_object
from reconstruct import read_json


class OmfScaffoldTests(unittest.TestCase):
    def test_existing_turbo_c_dgroup_topology_is_untouched(self):
        data = (ROOT / 'asm/F_652A.ASM').read_text()
        self.assertIn('DGROUP group _DATA,_BSS', data)
        report_path = ROOT / 'build/tlink-structural-report.json'
        if not report_path.exists():
            self.skipTest('structural TLINK probe has not produced F_652A')
        report = read_json(report_path)
        entry = next(item for item in report['relocatable_scaffold']
                     if item.get('owner') == 'F_652A')
        work = Path(report['byte_comparison']['candidate']).parent.parent
        source = (work / 'compile' / 'WORK' / entry['object']).read_bytes()
        self.assertEqual(ensure_turbo_c_dgroup(source), source)

    def test_large_dgroup_records_preserve_all_bytes_and_publics(self):
        data = bytes(range(256)) * 55
        publics = {'_data_%04d' % i: ('_DATA', i * 17) for i in range(400)}
        blob = make_dgroup_scaffold(data, 37000, publics)
        self.assertTrue(all(len(body) <= 1003 for _, body in _records(blob)))
        module = OmfReader().read(blob)
        self.assertEqual(module.segment_bytes('_DATA'), data)
        self.assertEqual(module.segment_length('_BSS'), 37000)
        self.assertEqual({p['name']: (p['segment'], p['offset']) for p in module.publics}, publics)

    def test_padding_is_a_relocatable_text_contribution(self):
        module = OmfReader().read(make_text_padding(7), 'PAD.OBJ')
        self.assertEqual(module.segment_length('_TEXT'), 7)
        self.assertEqual(module.segment_bytes('_TEXT'), b'\0' * 7)

    def test_trim_preserves_prefix_and_fixup_free_extent(self):
        candidates = sorted((ROOT / 'build').glob('tlink-structural-*/compile/WORK/R0009.OBJ'))
        if not candidates:
            self.skipTest('structural TLINK probe has not produced a compiler object')
        source = candidates[-1].read_bytes()
        trimmed = trim_text_contribution(source, 6571)
        module = OmfReader().read(trimmed, 'RUNTIME_BLOCK.OBJ')
        self.assertEqual(module.segment_length('_TEXT'), 6571)
        self.assertEqual(module.segment_bytes('_TEXT'), OmfReader().read(source).segment_bytes('_TEXT')[:6571])

    def test_remove_public_preserves_object_bytes(self):
        candidates = sorted((ROOT / 'build').glob('tlink-structural-*/WORK/R0209.OBJ'))
        candidates = [path for path in candidates
                      if '_getkey' in {public['name'] for public in OmfReader().read(path.read_bytes()).publics}]
        if not candidates:
            self.skipTest('structural TLINK probe has not produced the duplicate-public object')
        source = candidates[-1].read_bytes()
        original = OmfReader().read(source)
        trimmed = OmfReader().read(remove_public(source, '_getkey'))
        self.assertEqual(trimmed.segment_bytes('_TEXT'), original.segment_bytes('_TEXT'))
        self.assertEqual(trimmed.fixups, original.fixups)
        self.assertNotIn('_getkey', {public['name'] for public in trimmed.publics})

    def test_explicit_fixupp_order_changes_only_encounter_order(self):
        document = {'format': 'u16-farptr-u8-farptr-u8-tail8-v1', 'records': [
            {'word': 0, 'pointer_a': {'target': 'A', 'addend': 0}, 'byte_a': 0,
             'pointer_b': {'target': 'B', 'addend': 0}, 'byte_b': 0,
             'tail': [0] * 8}]}
        source = records_object(document, 'TABLE')
        ordered = order_explicit_fixupp_subrecords(source, '_DATA', descending=True)
        before, after = OmfReader().read(source), OmfReader().read(ordered)
        self.assertEqual([f['offset'] for f in before.fixups], [2, 7])
        self.assertEqual([f['offset'] for f in after.fixups], [7, 2])
        self.assertEqual(before.segment_bytes('_DATA'), after.segment_bytes('_DATA'))
        self.assertEqual(sorted(before.fixups, key=repr), sorted(after.fixups, key=repr))

    def test_standalone_tasm_dgroup_normalization_preserves_text_semantics(self):
        report_path = ROOT / 'build/tlink-structural-report.json'
        if not report_path.exists():
            self.skipTest('structural TLINK probe has not produced a TASM object')
        report = read_json(report_path)
        entry = next((entry for entry in report['relocatable_scaffold']
                      if entry.get('transforms') == ['Turbo C-compatible empty DGROUP metadata']), None)
        if entry is None:
            self.skipTest('structural TLINK probe has not staged a standalone TASM object')
        work = Path(report['byte_comparison']['candidate']).parent.parent
        source = work / 'compile' / 'WORK' / entry['object']
        before = OmfReader().read(source.read_bytes())
        after = OmfReader().read(ensure_turbo_c_dgroup(source.read_bytes()))
        self.assertEqual(after.segment_bytes('_TEXT'), before.segment_bytes('_TEXT'))
        self.assertEqual(after.publics_in('_TEXT'), before.publics_in('_TEXT'))
        self.assertEqual(after.externals, before.externals)
        self.assertEqual(after.fixups_in('_TEXT'), before.fixups_in('_TEXT'))
        self.assertEqual(after.segment_length('_DATA'), 0)
        self.assertEqual(after.segment_length('_BSS'), 0)
        self.assertEqual(next(group for group in after.groups if group['name'] == 'DGROUP')['segments'],
                         ['_BSS', '_DATA'])


if __name__ == '__main__':
    unittest.main()
