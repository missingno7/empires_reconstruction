"""Fresh complete-extent proof for the recovered F_53BF display-mode routine."""
from pathlib import Path
import sys
import tempfile
import unittest
ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from mz import MZ
from dos_runner import resolve_runner
from reconstruct import read_json, compile_sources, read_object, bind_region, mismatch, owned_library_modules
class MatchingCWave65Tests(unittest.TestCase):
    def test_recovered_complete_extent_and_fixups(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        owner = next(r for r in manifest['regions'] if r['id'] == 'F_53BF')
        # F_53BF (the video-detect display-mode routine) is now recovered as
        # exact C rather than symbolic ASM: it and F_50D2 were folded into the
        # src/STARTUP.C translation-unit merge (module C_4F63_520A in
        # layout/production-plan.json; see docs/current/asm-provenance.json).
        # The grouped ASM module M_50D2_53BF this test used to compile no
        # longer exists in layout/production-plan.json, so compile the
        # current C unit the region's own 'source'/'build' name, exactly like
        # any other single-region C member.
        self.assertEqual(owner['kind'], 'MATCHING_C')
        self.assertEqual(owner['source'], 'src/STARTUP.C')
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        lock = read_json(ROOT / 'layout/toolchain.json')
        modules = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            receipts, _ = compile_sources(ROOT, [owner], Path(temporary), ROOT / 'toolchain',
                                          resolve_runner(lock), lock)
            module = read_object((Path(temporary) / receipts[owner['id']]['object']).read_bytes())
            data, proof = bind_region(owner, module, MZ.parse(original), manifest['frames'], manifest['regions'], modules)
            mismatch(original[owner['start']:owner['end']], data, owner)
            self.assertEqual(len(data), 75)
            self.assertEqual(len(proof['fixups']), 6)
            self.assertEqual(proof['load_relocations'], [])
            self.assertEqual(data[:6], bytes.fromhex('c7 06 78 17 00 00'))
            self.assertEqual(data[-7:], bytes.fromhex('c7 06 78 17 03 00 c3'))
if __name__ == '__main__':
    unittest.main()
