from pathlib import Path
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from mz import MZ
from dos_runner import resolve_runner
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
                recipe_owner = next(r for r in recipe['owners'] if r['id'] == ident)
                # F_6036 and F_6181 were reverted to symbolic ASM after the
                # refactor's asm-origin review (see
                # docs/current/asm-origin-review.json); their recipes still
                # record their earlier MATCHING_C form. Only the ownership
                # facts this test exercises (kind, source, extent) need to
                # track the current truth. Both members were later
                # consolidated with F_60A9 into one word-alignment-proven
                # TASM module (asm/SPRDRAW.ASM, id M_6036_6181).
                self.assertEqual(owner['kind'], 'MATCHING_ASM')
                self.assertEqual(owner['source'], 'asm/SPRDRAW.ASM')
                self.assertEqual(owner['start'], recipe_owner['start'])
                self.assertEqual(owner['end'], recipe_owner['end'])
                work = Path(temporary) / ident
                work.mkdir()
                receipts, _ = compile_sources(ROOT, [owner], work, ROOT / 'toolchain',
                                               resolve_runner(lock), lock)
                module = read_object((work / receipts[ident]['object']).read_bytes())
                data, proof = bind_region(owner, module, MZ.parse(original), manifest['frames'],
                                          manifest['regions'], modules)
                mismatch(original[owner['start']:owner['end']], data, owner)
                self.assertEqual(len(data), length)
                self.assertEqual(len(proof['fixups']), fixups)
                self.assertEqual(proof['load_relocations'], reloc)


if __name__ == '__main__':
    unittest.main()
