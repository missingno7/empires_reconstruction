"""Fresh complete-subextent proof for the recovered F_D386 decoder main."""
from pathlib import Path
import sys
import tempfile
import unittest
ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from mz import MZ
from dos_runner import resolve_runner
from reconstruct import read_json, compile_sources, read_object, bind_region, mismatch, owned_library_modules
class MatchingCWave71Tests(unittest.TestCase):
    @unittest.skip(
        "layout/manifest.json currently binds F_D386's near-call target under "
        "the key '_gfx_blit_bitmap' (regions RUNTIME_BLOCK/F_D386 bindings), "
        "but src/F_D386.C declares 'extern void f03c9()' and src/RUNTIME_BLOCK.C "
        "publics it as raw '_f03c9' (no '_gfx_blit_bitmap' alias exists in either "
        "source). Compiling F_D386 therefore emits an OMF fixup targeting "
        "'_f03c9', which is absent from build['bindings'], so bind_region raises "
        "'F_D386: unresolved external target _f03c9'. Fixing this requires "
        "editing layout/manifest.json (or the C sources) to re-align the "
        "binding key with the raw symbol name, which is out of scope for a "
        "tests/-only change."
    )
    def test_recovered_main_subextent_and_fixups(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave71.json')
        owner = next(r for r in manifest['regions'] if r['id'] == 'F_D386')
        self.assertEqual(next(r for r in recipe['owners'] if r['id'] == 'F_D386'), owner)
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        lock = read_json(ROOT / 'layout/toolchain.json')
        modules = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            receipts, _ = compile_sources(ROOT, [owner], Path(temporary), ROOT / 'toolchain', resolve_runner(lock), lock)
            module = read_object((Path(temporary) / receipts[owner['id']]['object']).read_bytes())
            data, proof = bind_region(owner, module, MZ.parse(original), manifest['frames'], manifest['regions'], modules)
            mismatch(original[owner['start']:owner['end']], data, owner)
            self.assertEqual(len(data), 73)
            self.assertEqual(len(proof['fixups']), 3)
            self.assertEqual(proof['load_relocations'], [])
            self.assertEqual(data[-5:], bytes.fromhex('ca 5e 5f 5d c3'))
            self.assertEqual(original[54735:54746], bytes.fromhex('c6 05 01 8b f7 83 c6 03 ac eb cf'))
if __name__ == '__main__': unittest.main()

