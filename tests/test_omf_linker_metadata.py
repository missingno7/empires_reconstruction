import unittest
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from omf import OmfReader


TC20_C0C = Path(r'D:/Games/DOS/dos_recosystem/empires_forged/toolchain/dos/TC/LIB/C0C.OBJ')


class OmfLinkerMetadataTests(unittest.TestCase):
    @unittest.skipUnless(TC20_C0C.exists(), 'local pinned Turbo C C0C.OBJ is unavailable')
    def test_compact_startup_exposes_linker_attributes(self):
        module = OmfReader().read(TC20_C0C.read_bytes(), TC20_C0C.name)
        text = next(s for s in module.segment_defs if s['name'] == '_TEXT')
        self.assertEqual(text['class'], 'CODE')
        self.assertEqual(text['alignment'], 'byte')
        self.assertEqual(text['combine'], 'public')
        self.assertFalse(text['big'])
        self.assertFalse(text['use_32bit_offset'])
        dgroup = next(g for g in module.groups if g['name'] == 'DGROUP')
        self.assertEqual(dgroup['segments'][-1], '_DATA')
        self.assertEqual(dgroup['segments'][0], '_BSSEND')

    def test_segment_metadata_is_retained_for_library_module(self):
        library = ROOT / 'toolchain/CC.LIB'
        if not library.exists():
            self.skipTest('local pinned CC.LIB is unavailable')
        name, blob = OmfReader().split_library(library.read_bytes())[0]
        module = OmfReader().read(blob, name)
        self.assertTrue(module.segment_defs)
        self.assertEqual(module.segment_defs[0]['name'], '_TEXT')
        self.assertIn(module.segment_defs[0]['combine'], {'public', 'private', 'stack', 'common'})
        self.assertIsInstance(module.groups, list)
        self.assertIsInstance(module.comments, list)


if __name__ == '__main__':
    unittest.main()
