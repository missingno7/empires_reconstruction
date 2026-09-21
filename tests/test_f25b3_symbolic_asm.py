import tempfile
import unittest
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))

from mz import MZ
from dos_runner import resolve_runner
from reconstruct import (bind_region, compile_sources, mismatch, owned_library_modules,
                         read_json, read_object)


class F25B3SymbolicAssemblyTests(unittest.TestCase):
    def test_symbolic_module_is_exact_and_nonrelocating(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        owner = next(item for item in manifest['regions'] if item['id'] == 'F_25B3')
        # F_25B3 was recovered as exact C; see docs/current/exact-c-recovery.md.
        # It was later folded into the src/BOARD.C translation-unit merge
        # (module C_200F_3986 in layout/production-plan.json; see
        # docs/current/asm-provenance.json), so its old standalone
        # src/BOARDDRW.C is gone.
        self.assertEqual(owner['kind'], 'MATCHING_C')
        self.assertEqual(owner['source'], 'src/BOARD.C')
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        lock = read_json(ROOT / 'layout/toolchain.json')
        libraries = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            receipts, _ = compile_sources(ROOT, [owner], Path(temporary), ROOT / 'toolchain',
                                           resolve_runner(lock), lock)
            module = read_object((Path(temporary) / receipts['F_25B3']['object']).read_bytes())
            # F_25B3 now shares its src/BOARD.C translation unit with
            # several neighbouring functions (see
            # docs/current/exact-c-recovery.md), so the compiled module's
            # publics/fixups cover the whole file, not just this owner.
            self.assertIn('_board_update_moving_records', [p['name'] for p in module.publics])
            data, proof = bind_region(owner, module, MZ.parse(original), manifest['frames'],
                                      manifest['regions'], libraries)
            # The C recovery references shared globals/helpers by external fixup
            # instead of the ASM version's purely local addressing; all fixups
            # bound within this owner's own extent remain externally bound (no
            # load-time relocations survive binding).
            self.assertEqual(len(proof['fixups']), 33)
            self.assertTrue(all(f['target_kind'] == 'external' for f in proof['fixups']))
        mismatch(original[owner['start']:owner['end']], data, owner)
        self.assertEqual(len(data), 761)
        self.assertEqual(proof['load_relocations'], [])


if __name__ == '__main__':
    unittest.main()
