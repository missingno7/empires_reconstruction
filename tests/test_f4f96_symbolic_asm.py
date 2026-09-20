import tempfile
import unittest
from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))

from mz import MZ
from reconstruct import (bind_region, compile_sources, mismatch,
                         owned_library_modules, read_json, read_object)


class F4F96SymbolicAssemblyTests(unittest.TestCase):
    def test_symbolic_option_parser_is_exact_and_nonrelocating(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        owner = next(item for item in manifest['regions'] if item['id'] == 'F_4F96')
        self.assertEqual((owner['kind'], owner['source']),
                         ('MATCHING_ASM', 'asm/F_4F96.ASM'))
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        lock = read_json(ROOT / 'layout/toolchain.json')
        libraries = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            receipts, _ = compile_sources(ROOT, [owner], Path(temporary), ROOT / 'toolchain',
                                           Path(lock['dosbox_default']), lock)
            module = read_object((Path(temporary) / receipts['F_4F96']['object']).read_bytes())
            data, proof = bind_region(owner, module, MZ.parse(original), manifest['frames'],
                                      manifest['regions'], libraries)
        self.assertEqual(module.publics, [{'name': '_f4f96', 'segment': '_TEXT', 'offset': 0}])
        self.assertEqual(module.fixups, [])
        mismatch(original[owner['start']:owner['end']], data, owner)
        self.assertEqual(len(data), 299)
        self.assertEqual(proof['load_relocations'], [])


if __name__ == '__main__':
    unittest.main()
