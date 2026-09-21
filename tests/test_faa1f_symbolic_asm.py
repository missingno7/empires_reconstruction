import tempfile
import unittest
from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))

from mz import MZ
from dos_runner import resolve_runner
from reconstruct import (bind_region, compile_sources, mismatch,
                         owned_library_modules, read_json, read_object)


class FAA1FSymbolicAssemblyTests(unittest.TestCase):
    def test_symbolic_dispatch_loop_is_exact(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        owner = next(item for item in manifest['regions'] if item['id'] == 'F_AA1F')
        # F_AA1F was recovered as exact C; see docs/history/exact-c-recovery.md.
        self.assertEqual((owner['kind'], owner['source']),
                         ('MATCHING_C', 'src/SLOTMENU.C'))
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        lock = read_json(ROOT / 'layout/toolchain.json')
        libraries = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            receipts, _ = compile_sources(ROOT, [owner], Path(temporary), ROOT / 'toolchain',
                                           resolve_runner(lock), lock)
            module = read_object((Path(temporary) / receipts['F_AA1F']['object']).read_bytes())
            data, proof = bind_region(owner, module, MZ.parse(original), manifest['frames'],
                                      manifest['regions'], libraries)
        mismatch(original[owner['start']:owner['end']], data, owner)
        self.assertEqual(len(data), 327)
        # F_AA1F now shares its src/SLOTMENU.C translation unit with several
        # neighbouring functions (see docs/history/exact-c-recovery.md), so
        # module.fixups covers the whole file; only proof['fixups'] is scoped
        # to this owner's own extent.
        self.assertEqual(len(proof['fixups']), 33)
        self.assertEqual(proof['load_relocations'], [])


if __name__ == '__main__':
    unittest.main()
