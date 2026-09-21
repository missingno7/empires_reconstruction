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


class F880ASymbolicAssemblyTests(unittest.TestCase):
    def test_symbolic_state_loop_is_exact(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        owner = next(item for item in manifest['regions'] if item['id'] == 'F_880A')
        # F_880A was recovered as exact C (absorbing former F_8C04); see
        # docs/current/exact-c-recovery.md.
        self.assertEqual((owner['kind'], owner['source']),
                         ('MATCHING_C', 'src/DIALOG.C'))
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        lock = read_json(ROOT / 'layout/toolchain.json')
        libraries = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            receipts, _ = compile_sources(ROOT, [owner], Path(temporary), ROOT / 'toolchain',
                                           resolve_runner(lock), lock)
            module = read_object((Path(temporary) / receipts['F_880A']['object']).read_bytes())
            data, proof = bind_region(owner, module, MZ.parse(original), manifest['frames'],
                                      manifest['regions'], libraries)
        mismatch(original[owner['start']:owner['end']], data, owner)
        self.assertEqual(len(data), 557)
        # F_880A now shares its src/DIALOG.C translation unit with several
        # neighbouring functions (see docs/current/exact-c-recovery.md), so
        # module.fixups covers the whole file; proof['fixups'] is the count
        # scoped to this owner's own extent.
        self.assertEqual(len(proof['fixups']), 29)
        self.assertEqual(proof['load_relocations'], [])


if __name__ == '__main__':
    unittest.main()
