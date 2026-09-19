"""Fresh complete-extent proofs for the recovered timer-vector C routines."""
from pathlib import Path
import sys
import tempfile
import unittest
ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from mz import MZ
from reconstruct import read_json, compile_sources, read_object, bind_region, mismatch, owned_library_modules
class MatchingCWave66Tests(unittest.TestCase):
    def test_recovered_complete_extents_and_fixups(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave66.json')
        owners = [next(r for r in manifest['regions'] if r['id'] == i) for i in ('F_6B7A','F_6BAC')]
        for owner in owners: self.assertEqual(next(r for r in recipe['owners'] if r['id'] == owner['id']), owner)
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        lock = read_json(ROOT / 'layout/toolchain.json')
        modules = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            receipts, _ = compile_sources(ROOT, owners, Path(temporary), ROOT / 'toolchain', Path(lock['dosbox_default']), lock)
            for owner in owners:
                module = read_object((Path(temporary) / receipts[owner['id']]['object']).read_bytes())
                data, proof = bind_region(owner, module, MZ.parse(original), manifest['frames'], manifest['regions'], modules)
                mismatch(original[owner['start']:owner['end']], data, owner)
                self.assertEqual(len(data), owner['end'] - owner['start'])
                self.assertEqual(len(proof['fixups']), 2 if owner['id']=='F_6B7A' else 2)
                self.assertEqual(proof['load_relocations'], [])
if __name__ == '__main__': unittest.main()
