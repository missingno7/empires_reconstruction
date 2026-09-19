"""Regression proof for the normalized void declarations in F_9DCC."""
from pathlib import Path
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))

from mz import MZ
from reconstruct import (bind_region, compile_sources, mismatch, owned_library_modules,
                         read_json, read_object)


class SharedInterfaceF9DccTests(unittest.TestCase):
    def test_void_callee_declarations_preserve_complete_owner(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        owner = next(item for item in manifest['regions'] if item['id'] == 'F_9DCC')
        lock = read_json(ROOT / 'layout/toolchain.json')
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            work = Path(temporary)
            receipts, _ = compile_sources(ROOT, [owner], work, ROOT / 'toolchain',
                                            Path(lock['dosbox_default']), lock)
            module = read_object((work / receipts[owner['id']]['object']).read_bytes())
            data, proof = bind_region(owner, module, MZ.parse(original), manifest['frames'],
                                      manifest['regions'],
                                      owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock))
        mismatch(original[owner['start']:owner['end']], data, owner)
        self.assertEqual((len(data), len(proof['fixups'])), (247, 30))


if __name__ == '__main__':
    unittest.main()
