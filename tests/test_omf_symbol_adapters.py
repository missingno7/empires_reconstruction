import unittest
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from omf import OmfReader
from omf_scaffold import make_external_demand, rename_external


class OmfSymbolAdapterTests(unittest.TestCase):
    def test_rename_external_preserves_reference_indices(self):
        candidates = sorted((ROOT / 'build').glob('tlink-structural-*/compile/WORK/R0199.OBJ'))
        if not candidates:
            self.skipTest('structural TLINK probe has not produced the caller object')
        source = candidates[-1].read_bytes()
        renamed = rename_external(source, '_ff9be', '_toupper')
        module = OmfReader().read(renamed, 'F_A525.OBJ')
        self.assertNotIn('_ff9be', module.externals)
        self.assertIn('_toupper', module.externals)
        self.assertEqual(len(module.fixups_in('_TEXT')), len(OmfReader().read(source).fixups_in('_TEXT')))

    def test_external_demand_round_trips(self):
        module = OmfReader().read(make_external_demand(['_toupper', '_f6c57']))
        self.assertEqual(set(module.externals), {'_toupper', '_f6c57'})


if __name__ == '__main__':
    unittest.main()
