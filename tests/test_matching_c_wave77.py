from pathlib import Path
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from mz import MZ
from reconstruct import (bind_region, compile_sources, mismatch, owned_library_modules,
                         read_json, read_object)


class MatchingCWave77Tests(unittest.TestCase):
    def test_complete_extent_and_strlen_fixup(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave77.json')
        owner = next(r for r in manifest['regions'] if r['id'] == 'F_4F63')
        self.assertEqual(next(r for r in recipe['owners'] if r['id'] == 'F_4F63'), owner)
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
            self.assertEqual(len(data), 51)
            self.assertEqual(len(proof['fixups']), 1)
            self.assertEqual(proof['fixups'][0]['target'], '_strlen')
            self.assertEqual(proof['load_relocations'], [])
            self.assertEqual(data, original[owner['start']:owner['end']])


if __name__ == '__main__':
    unittest.main()
