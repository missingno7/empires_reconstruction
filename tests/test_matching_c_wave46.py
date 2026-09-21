"""Fresh complete-extent proof for the F_B967 render-loop wrapper."""
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


class MatchingCWave46Tests(unittest.TestCase):
    def test_fresh_render_loop_extent_and_fixups(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        owner = next(r for r in manifest['regions'] if r['id'] == 'F_B967')
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
            self.assertEqual(len(data), 56)
            self.assertEqual(len(proof['fixups']), 12)


if __name__ == '__main__':
    unittest.main()
