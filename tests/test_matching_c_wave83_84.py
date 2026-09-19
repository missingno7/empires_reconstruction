from pathlib import Path
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from mz import MZ
from reconstruct import bind_region, compile_sources, mismatch, owned_library_modules, read_json, read_object


class MatchingCWave83_84Tests(unittest.TestCase):
    def test_complete_sprite_pair(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        lock = read_json(ROOT / 'layout/toolchain.json')
        modules = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            for ident, recipe_name, length, fixups, reloc in [
                    ('F_6036', 'matching-wave83.json', 115, 6, [24683]),
                    ('F_6181', 'matching-wave84.json', 171, 7, [25049])]:
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
