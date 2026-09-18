"""Fresh complete-extent proof for the recovered F_E114 C routine."""
from pathlib import Path
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from mz import MZ
from reconstruct import (read_json, compile_sources, read_object, bind_region,
                         mismatch, owned_library_modules)


class MatchingCWave57Tests(unittest.TestCase):
    def test_recovered_complete_extent_and_fixups(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave57.json')
        owner = next(r for r in manifest['regions'] if r['id'] == 'F_E114')
        self.assertEqual(next(r for r in recipe['owners'] if r['id'] == 'F_E114'), owner)
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
            self.assertEqual(len(data), 61)
            self.assertEqual(len(proof['fixups']), 1)
            self.assertEqual(proof['load_relocations'], [])
            self.assertEqual(data[:6], bytes.fromhex('55 8b ec 83 ec 1c'))
            self.assertEqual(data[-6:], bytes.fromhex('08 5e 8b e5 5d c3'))


if __name__ == '__main__':
    unittest.main()
