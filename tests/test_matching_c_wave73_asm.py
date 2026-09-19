"""Fresh complete-extent proof for the recovered F_01A4 ASM setup."""
from pathlib import Path
import sys
import tempfile
import unittest
ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from mz import MZ
from reconstruct import read_json, compile_sources, read_object, bind_region, mismatch, owned_library_modules
class MatchingCWave73AsmTests(unittest.TestCase):
    def test_recovered_setup_extent_and_fixups(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave73.json')
        owner = next(r for r in manifest['regions'] if r['id'] == 'F_01A4')
        historical = next(r for r in recipe['owners'] if r['id'] == 'F_01A4')
        self.assertEqual((historical['start'], historical['end'], historical['expected_sha256']),
                         (owner['start'], owner['end'], owner['expected_sha256']))
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        lock = read_json(ROOT / 'layout/toolchain.json')
        modules = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            receipts, _ = compile_sources(ROOT, [owner], Path(temporary), ROOT / 'toolchain', Path(lock['dosbox_default']), lock)
            module = read_object((Path(temporary) / receipts[owner['id']]['object']).read_bytes())
            data, proof = bind_region(owner, module, MZ.parse(original), manifest['frames'], manifest['regions'], modules)
            mismatch(original[owner['start']:owner['end']], data, owner)
            self.assertEqual(data, bytes.fromhex('b9 1e 00 90 ba 3d 00 2e 8e 1e ba 01 e8 e9 ff b8 03 00 50 e8 4a ff'))
            self.assertEqual(len(proof['fixups']), 2)
            self.assertEqual(proof['load_relocations'], [])
if __name__ == '__main__': unittest.main()
