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
                    ('F_B7F9', 'matching-wave90.json', 366, 0, []),
                    ('F_B122', 'matching-wave91.json', 693, 0, []),
                    ('F_8BAB', 'matching-wave92.json', 1275, 0, []),
                    ('F_B99F', 'matching-wave93.json', 1857, 0, []),
                    ('F_C2EA', 'matching-wave94.json', 111, 0, []),
                    ('F_C359', 'matching-wave94.json', 130, 0, []),
                    ('F_C3DB', 'matching-wave94.json', 101, 0, []),
                    ('F_C440', 'matching-wave94.json', 193, 0, []),
                    ('F_C501', 'matching-wave94.json', 72, 0, []),
                    ('F_C549', 'matching-wave94.json', 30, 0, []),
                    ('F_C5A8', 'matching-wave94.json', 11, 0, []),
                    ('F_C5B3', 'matching-wave94.json', 19, 0, []),
                    ('F_C5C6', 'matching-wave94.json', 11, 0, []),
                    ('F_C5D1', 'matching-wave94.json', 167, 0, []),
                    ('F_C678', 'matching-wave94.json', 65, 0, []),
                    ('F_C6B9', 'matching-wave94.json', 77, 0, []),
                    ('F_C706', 'matching-wave94.json', 79, 0, []),
                    ('F_C755', 'matching-wave94.json', 37, 0, []),
                    ('F_C77A', 'matching-wave94.json', 81, 0, []),
                    ('F_C7CB', 'matching-wave94.json', 105, 0, []),
                    ('F_C9A4', 'matching-wave94.json', 95, 0, []),
                    ('F_CA03', 'matching-wave94.json', 50, 0, []),
                    ('F_CA35', 'matching-wave94.json', 28, 0, []),
                    ('F_CA51', 'matching-wave94.json', 50, 0, []),
                    ('F_C567', 'matching-wave95.json', 51, 0, []),
                    ('F_4F96', 'matching-wave96.json', 299, 0, []),
                    ('F_25B3', 'matching-wave97.json', 761, 0, []),
                    ('F_28AC', 'matching-wave97.json', 218, 0, []),
                    ('F_D61C', 'matching-wave97.json', 384, 0, []),
                    ('F_D79C', 'matching-wave97.json', 123, 0, []),
                    ('F_7964', 'matching-wave98.json', 623, 0, []),
                    ('F_880A', 'matching-wave99.json', 506, 0, []),
                    ('F_4B0C', 'matching-wave100.json', 915, 0, []),
                    ('F_7DD3', 'matching-wave101.json', 41, 0, []),
                    ('F_8C04', 'matching-wave101.json', 51, 0, []),
                    ('F_DF98', 'matching-wave103.json', 253, 2, [57266, 57344])]:
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
