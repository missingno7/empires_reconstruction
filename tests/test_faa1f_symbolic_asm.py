import tempfile
import unittest
from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))

from mz import MZ
from reconstruct import (bind_region, compile_sources, mismatch,
                         owned_library_modules, read_json, read_object)


class FAA1FSymbolicAssemblyTests(unittest.TestCase):
    def test_symbolic_dispatch_loop_is_exact(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        owner = next(item for item in manifest['regions'] if item['id'] == 'F_AA1F')
        self.assertEqual((owner['kind'], owner['source']),
                         ('MATCHING_ASM', 'asm/F_AA1F.ASM'))
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        lock = read_json(ROOT / 'layout/toolchain.json')
        libraries = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            receipts, _ = compile_sources(ROOT, [owner], Path(temporary), ROOT / 'toolchain',
                                           Path(lock['dosbox_default']), lock)
            module = read_object((Path(temporary) / receipts['F_AA1F']['object']).read_bytes())
            data, proof = bind_region(owner, module, MZ.parse(original), manifest['frames'],
                                      manifest['regions'], libraries)
        mismatch(original[owner['start']:owner['end']], data, owner)
        self.assertEqual(len(data), 327)
        self.assertEqual(len(proof['fixups']), len(module.fixups))
        self.assertEqual(len(module.fixups), 16)
        self.assertEqual(proof['load_relocations'], [])


if __name__ == '__main__':
    unittest.main()
