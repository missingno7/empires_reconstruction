"""Fresh complete-extent proof for the recovered F_CA9B C/inline-assembler routine."""
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


class MatchingCWave63Tests(unittest.TestCase):
    def test_recovered_complete_extent_and_fixups(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave63.json')
        owner = next(r for r in manifest['regions'] if r['id'] == 'F_CA9B')
        self.assertEqual(next(r for r in recipe['owners'] if r['id'] == 'F_CA9B'), owner)
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        lock = read_json(ROOT / 'layout/toolchain.json')
        modules = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        # F_CA9B now lives inside the single hand-written asm/SOUND.ASM module
        # (M_C1A0_CB48 in layout/production-plan.json, see
        # docs/current/asm-provenance.json). The manifest region's own
        # 'bindings' only lists the DGROUP words this member's disassembly
        # review called out; it does not carry _notetab, which this member's
        # code also reaches (`mov bx, offset _notetab`) and which the plan
        # module's aggregate bindings do declare. Bind against the plan
        # module's full bindings set, the way production build/probe_module.py
        # resolve a member of a grouped module, while still compiling and
        # measuring this member's own extent.
        plan = read_json(ROOT / 'layout/production-plan.json')['modules']
        module_plan = next(m for m in plan if m['id'] == 'M_C1A0_CB48')
        compile_owner = dict(owner)
        compile_owner['build'] = {**owner['build'], 'bindings': module_plan['build']['bindings']}
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            receipts, _ = compile_sources(ROOT, [compile_owner], Path(temporary), ROOT / 'toolchain',
                                          resolve_runner(lock), lock)
            module = read_object((Path(temporary) / receipts[compile_owner['id']]['object']).read_bytes())
            data, proof = bind_region(compile_owner, module, MZ.parse(original), manifest['frames'],
                                      manifest['regions'], modules)
            mismatch(original[owner['start']:owner['end']], data, owner)
            self.assertEqual(len(data), 53)
            # Calls to _speaker_gate_off/_speaker_gate_on/_pit_channel2_set_divisor
            # are now intra-module near calls TASM resolves at assemble time (no
            # OMF fixup); only the three data-table/variable references remain.
            self.assertEqual(len(proof['fixups']), 3)
            self.assertEqual(proof['load_relocations'], [])
            self.assertEqual(data[:7], bytes.fromhex('55 8b ec 26 8a 55 01'))
            self.assertEqual(data[-8:], bytes.fromhex('a1 92 1e a3 90 1e 5d c3'))


if __name__ == '__main__':
    unittest.main()

