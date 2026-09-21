"""Fresh TASM proof for the F_CA51 symbolic scale-and-store routine."""
from pathlib import Path
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from mz import MZ
from dos_runner import resolve_runner
from reconstruct import (read_json, compile_sources, read_object, bind_region,
                         mismatch, owned_library_modules)


class SymbolicAsmWave148Tests(unittest.TestCase):
    def test_f_ca51_has_complete_extent_and_real_data_fixups(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave148.json')
        owner = next(region for region in manifest['regions'] if region['id'] == 'F_CA51')
        # F_CA51 now lives inside the single hand-written asm/SOUND.ASM module
        # (M_C1A0_CB48; see docs/current/asm-provenance.json), not the
        # standalone recovery/asm/F_CA51.ASM this recipe recorded.
        self.assertEqual(owner['kind'], 'MATCHING_ASM')
        self.assertEqual(owner['source'], 'asm/SOUND.ASM')
        self.assertEqual(owner['end'] - owner['start'], 50)
        self.assertEqual(recipe['format'], 'empires-symbolic-asm-promotion-v1')
        self.assertEqual(recipe['conversions'][0]['id'], 'F_CA51')
        self.assertNotIn('\ndb ', (ROOT / owner['source']).read_text().lower())

        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        lock = read_json(ROOT / 'layout/toolchain.json')
        modules = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            receipts, _ = compile_sources(ROOT, [owner], Path(temporary), ROOT / 'toolchain',
                                          resolve_runner(lock), lock)
            module = read_object((Path(temporary) / receipts[owner['id']]['object']).read_bytes())
            data, proof = bind_region(owner, module, MZ.parse(original), manifest['frames'],
                                      manifest['regions'], modules)
        mismatch(original[owner['start']:owner['end']], data, owner)
        # 'offset' is now the fixup's absolute position in the much larger
        # shared asm/SOUND.ASM segment; 'extent_offset' (position within this
        # 50-byte extracted region) is the position-invariant quantity this
        # test originally pinned.
        self.assertEqual([(fixup['extent_offset'], fixup['target']) for fixup in proof['fixups']],
                         [(15, '_g1e84'), (38, '_g1e86'), (44, '_g1e86')])
        self.assertEqual(proof['load_relocations'], [])


if __name__ == '__main__':
    unittest.main()
