"""Fresh complete-extent proof for the symbolic F_C8D4 port wrapper."""
from pathlib import Path
import sys
import tempfile
import unittest
ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from mz import MZ
from dos_runner import resolve_runner
from reconstruct import read_json, compile_sources, read_object, bind_region, mismatch, owned_library_modules
class MatchingCWave67Tests(unittest.TestCase):
    def test_recovered_complete_extent_and_fixup(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave67.json')
        owner = next(r for r in manifest['regions'] if r['id'] == 'F_C8D4')
        self.assertEqual(next(r for r in recipe['owners'] if r['id'] == 'F_C8D4'), owner)
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        lock = read_json(ROOT / 'layout/toolchain.json')
        modules = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            receipts, _ = compile_sources(ROOT, [owner], Path(temporary), ROOT / 'toolchain', resolve_runner(lock), lock)
            module = read_object((Path(temporary) / receipts[owner['id']]['object']).read_bytes())
            data, proof = bind_region(owner, module, MZ.parse(original), manifest['frames'], manifest['regions'], modules)
            mismatch(original[owner['start']:owner['end']], data, owner)
            self.assertEqual(data, bytes.fromhex('55 8b ec 52 8b 16 30 18 ee eb 00 5a 5d c3'))
            self.assertEqual(len(proof['fixups']), 1)
            self.assertEqual(proof['load_relocations'], [])
if __name__ == '__main__': unittest.main()
