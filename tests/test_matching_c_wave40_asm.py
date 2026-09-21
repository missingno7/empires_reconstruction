"""Fresh TASM proof for the final held linkage extent."""
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


class MatchingCWave40AsmTests(unittest.TestCase):
    def test_fresh_asm_extent_and_bindings(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        owner = next(r for r in manifest['regions'] if r['id'] == 'F_4E9F')
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
            # F_4E9F was consolidated with F_4AA8/F_4B0C/F_4EEB into one
            # word-alignment-proven TASM module (asm/SPRITES.ASM, id
            # M_4AA8_4EEB); its call into _play_window_wipe_clipped (F_4AA8)
            # is now an intra-module backward reference TASM resolves
            # directly, with no linker fixup needed, so the standalone count
            # drops from 5 to 4 (the remaining four DGROUP-offset fixups).
            self.assertEqual(len(proof['fixups']), 4)


if __name__ == '__main__':
    unittest.main()
