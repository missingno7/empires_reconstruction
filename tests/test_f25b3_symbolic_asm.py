import tempfile
import unittest
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))

from mz import MZ
from reconstruct import (bind_region, compile_sources, mismatch, owned_library_modules,
                         read_json, read_object)


class F25B3SymbolicAssemblyTests(unittest.TestCase):
    def test_symbolic_module_is_exact_and_nonrelocating(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        owner = next(item for item in manifest['regions'] if item['id'] == 'F_25B3')
        self.assertEqual(owner['kind'], 'MATCHING_ASM')
        self.assertEqual(owner['source'], 'asm/F_25B3.ASM')
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        lock = read_json(ROOT / 'layout/toolchain.json')
        libraries = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            receipts, _ = compile_sources(ROOT, [owner], Path(temporary), ROOT / 'toolchain',
                                           Path(lock['dosbox_default']), lock)
            module = read_object((Path(temporary) / receipts['F_25B3']['object']).read_bytes())
            self.assertEqual(module.publics, [{'name': '_f_25b3', 'segment': '_TEXT', 'offset': 0}])
            self.assertEqual(module.fixups, [])
            data, proof = bind_region(owner, module, MZ.parse(original), manifest['frames'],
                                      manifest['regions'], libraries)
        mismatch(original[owner['start']:owner['end']], data, owner)
        self.assertEqual(len(data), 761)
        self.assertEqual(proof['load_relocations'], [])


if __name__ == '__main__':
    unittest.main()
