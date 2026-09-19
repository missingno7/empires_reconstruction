from pathlib import Path
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from mz import MZ
from reconstruct import bind_region, compile_sources, mismatch, owned_library_modules, read_json, read_object


class MatchingCWave85_86Tests(unittest.TestCase):
    def test_complete_extents_and_relocation_proofs(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        lock = read_json(ROOT / 'layout/toolchain.json')
        modules = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            for ident, recipe_name, length, fixups, reloc in [
                    ('F_9EC3', 'matching-wave85.json', 125, 4, [40700, 40712]),
                    ('F_643A', 'matching-wave86.json', 240, 26, []),
                    ('F_AA1F', 'matching-wave87.json', 327, 0, []),
                    ('F_AB66', 'matching-wave88.json', 385, 0, []),
                    ('F_B40F', 'matching-wave89.json', 236, 0, []),
                    ('F_B7F9', 'matching-wave90.json', 366, 0, [])]:
                owner = next(r for r in manifest['regions'] if r['id'] == ident)
                recipe = read_json(ROOT / 'recipes/c' / recipe_name)
                self.assertEqual(next(r for r in recipe['owners'] if r['id'] == ident), owner)
                work = Path(temporary) / ident
                work.mkdir()
                receipts, _ = compile_sources(ROOT, [owner], work, ROOT / 'toolchain',
                                               Path(lock['dosbox_default']), lock)
                module = read_object((work / receipts[ident]['object']).read_bytes())
                data, proof = bind_region(owner, module, MZ.parse(original), manifest['frames'],
                                          manifest['regions'], modules)
                mismatch(original[owner['start']:owner['end']], data, owner)
                self.assertEqual(len(data), length)
                self.assertEqual(len(proof['fixups']), fixups)
                self.assertEqual(proof['load_relocations'], reloc)


if __name__ == '__main__':
    unittest.main()
