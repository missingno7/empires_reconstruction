"""Fresh complete-extent proof for the three recovered F_D45C wrappers."""
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


class MatchingCWave55Tests(unittest.TestCase):
    def test_recovered_complete_extents_and_fixups(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave55.json')
        ids = ('F_D471', 'F_D487', 'F_D49D')
        owners = [next(r for r in manifest['regions'] if r['id'] == ident) for ident in ids]
        self.assertEqual([next(r for r in recipe['owners'] if r['id'] == ident) for ident in ids], owners)
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        lock = read_json(ROOT / 'layout/toolchain.json')
        modules = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            receipts, _ = compile_sources(ROOT, owners, Path(temporary), ROOT / 'toolchain',
                                          resolve_runner(lock), lock)
            for owner in owners:
                module = read_object((Path(temporary) / receipts[owner['id']]['object']).read_bytes())
                data, proof = bind_region(owner, module, MZ.parse(original), manifest['frames'],
                                          manifest['regions'], modules)
                mismatch(original[owner['start']:owner['end']], data, owner)
                self.assertEqual(len(data), 22)
                # dialog_list_pick is now a call inside src/HELPMENU.C (no fixup); the title string stays external.
                self.assertEqual(len(proof['fixups']), 1)
                self.assertEqual(proof['load_relocations'], [])
                self.assertEqual(data[:2], bytes.fromhex('1e b8'))
                self.assertEqual(data[-3:], bytes.fromhex('eb 00 c3'))


if __name__ == '__main__':
    unittest.main()
