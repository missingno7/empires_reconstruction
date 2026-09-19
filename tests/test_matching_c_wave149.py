"""Fresh TASM proof for the F_7DD3 cleanup continuation."""
from pathlib import Path
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from mz import MZ
from reconstruct import (read_json, compile_sources, read_object, bind_region,
                         mismatch, owned_library_modules)


class SymbolicAsmWave149Tests(unittest.TestCase):
    def test_f_7dd3_has_complete_extent_and_real_call_fixups(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave149.json')
        owner = next(region for region in manifest['regions'] if region['id'] == 'F_7DD3')
        self.assertEqual(owner['kind'], 'MATCHING_ASM')
        self.assertEqual(owner['source'], 'asm/F_7DD3.ASM')
        self.assertEqual(owner['end'] - owner['start'], 41)
        self.assertEqual(recipe['format'], 'empires-symbolic-asm-promotion-v1')
        self.assertEqual(recipe['conversions'][0]['id'], 'F_7DD3')
        self.assertNotIn('\ndb ', (ROOT / owner['source']).read_text().lower())

        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        lock = read_json(ROOT / 'layout/toolchain.json')
        modules = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            receipts, _ = compile_sources(ROOT, [owner], Path(temporary), ROOT / 'toolchain',
                                          Path(lock['dosbox_default']), lock)
            module = read_object((Path(temporary) / receipts[owner['id']]['object']).read_bytes())
            data, proof = bind_region(owner, module, MZ.parse(original), manifest['frames'],
                                      manifest['regions'], modules)
        mismatch(original[owner['start']:owner['end']], data, owner)
        self.assertEqual([(fixup['offset'], fixup['target']) for fixup in proof['fixups']],
                         [(5, '_f7bfc'), (15, '_f791e'), (18, '_fd5a6'),
                          (21, '_f7162'), (30, '_f6997'), (33, '_f6b66')])
        self.assertEqual(proof['load_relocations'], [])


if __name__ == '__main__':
    unittest.main()
