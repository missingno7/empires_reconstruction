import unittest
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from omf import OmfReader
from data_omf import emit_data
from omf_scaffold import (make_dgroup_scaffold, make_external_demand,
                          normalize_external_case, rename_external,
                          rename_external_addend)


class OmfSymbolAdapterTests(unittest.TestCase):
    def test_rename_external_preserves_reference_indices(self):
        source = emit_data(bytes(4), {'pointer': 0}, [{'offset': 0, 'target': '_ff9be'}], 'ADAPTER_TEST')
        renamed = rename_external(source, '_ff9be', '_toupper')
        module = OmfReader().read(renamed, 'F_A525.OBJ')
        self.assertNotIn('_ff9be', module.externals)
        self.assertIn('_toupper', module.externals)
        self.assertEqual(len(module.fixups_in('_DATA')), len(OmfReader().read(source).fixups_in('_DATA')))

    def test_external_demand_round_trips(self):
        module = OmfReader().read(make_external_demand(['_toupper', '_f6c57']))
        self.assertEqual(set(module.externals), {'_toupper', '_f6c57'})

    def test_dgroup_scaffold_exposes_grouped_publics(self):
        module = OmfReader().read(make_dgroup_scaffold(
            b'abc', 12, {'_g1': ('_DATA', 1), '_g2': ('_BSS', 3)}))
        self.assertEqual(module.segment_length('_DATA'), 3)
        self.assertEqual(module.segment_length('_BSS'), 12)
        self.assertEqual({public['name'] for public in module.publics}, {'_g1', '_g2'})
        self.assertEqual(module.groups[0]['segments'], ['_BSS', '_DATA'])

    def test_case_normalization_requires_an_explicit_public(self):
        source = make_external_demand(['_F00D', '_OTHER'])
        normalized = normalize_external_case(source, {'_f00d'})
        module = OmfReader().read(normalized)
        self.assertIn('_f00d', module.externals)
        self.assertIn('_OTHER', module.externals)

    def test_rename_external_addend_preserves_fixup_and_updates_displacement(self):
        source = emit_data(bytes(4), {'pointer': 0}, [{'offset': 0, 'target': '_ff9be'}], 'ADAPTER_TEST')
        renamed = rename_external_addend(source, '_ff9be', '_toupper', 3)
        before = OmfReader().read(source)
        after = OmfReader().read(renamed)
        self.assertNotIn('_ff9be', after.externals)
        self.assertIn('_toupper', after.externals)
        self.assertEqual(len(before.fixups_in('_DATA')), len(after.fixups_in('_DATA')))
        self.assertEqual(after.segment_bytes('_DATA'), bytes([3, 0, 0, 0]))


if __name__ == '__main__':
    unittest.main()
