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
        # F_53BF was reverted to symbolic ASM and now belongs to the grouped
        # module M_50D2_53BF (see layout/production-plan.json and
        # docs/current/asm-origin-review.json), which calls the renamed
        # _opl_detect (was _fe54d). The standalone recovery/src/F_53BF.ASM this
        # manifest region's 'source' field names is a stale leftover that
        # still externs the pre-rename _fe54d and no longer matches the
        # binding manifest['regions'] declares for this owner; compile and
        # bind the grouped module instead, the way the production build does.
        self.assertEqual(owner['kind'], 'MATCHING_ASM')
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        lock = read_json(ROOT / 'layout/toolchain.json')
        modules = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        plan = read_json(ROOT / 'layout/production-plan.json')['modules']
        module_plan = next(m for m in plan if m['id'] == 'M_50D2_53BF')
        self.assertIn('F_53BF', module_plan['members'])
        group_owner = {'id': module_plan['id'], 'kind': 'MATCHING_ASM', 'source': module_plan['source'],
                       'build': {'flags_append': ''}}
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            receipts, _ = compile_sources(ROOT, [group_owner], Path(temporary), ROOT / 'toolchain',
                                          resolve_runner(lock), lock)
            module = read_object((Path(temporary) / receipts[group_owner['id']]['object']).read_bytes())
            data, proof = bind_region(owner, module, MZ.parse(original), manifest['frames'], manifest['regions'], modules)
            mismatch(original[owner['start']:owner['end']], data, owner)
            self.assertEqual(len(data), 75)
            self.assertEqual(len(proof['fixups']), 6)
            self.assertEqual(proof['load_relocations'], [])
            self.assertEqual(data[:6], bytes.fromhex('c7 06 78 17 00 00'))
            self.assertEqual(data[-7:], bytes.fromhex('c7 06 78 17 03 00 c3'))
if __name__ == '__main__':
    unittest.main()
