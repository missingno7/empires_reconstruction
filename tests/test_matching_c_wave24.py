"""Full fresh comparisons and expression/layout mutants for the twenty-fourth C wave."""
from pathlib import Path
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from mz import MZ
from dos_runner import resolve_runner
from reconstruct import (bind_region, compile_sources, mismatch, owned_library_modules,
                         read_json, read_object)


class MatchingCWave24Tests(unittest.TestCase):
    def test_fresh_wave_and_expression_layout_mutants(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        lock = read_json(ROOT / 'layout/toolchain.json')
        modules = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        # F_90A6 and F_969D both now belong to the grouped multi-source module
        # C_8A37_969D (layout/production-plan.json): each relies on GC316.H,
        # which only F_8A37 (an earlier member) includes. Standalone
        # compilation of either file alone now fails, so compile the whole
        # module's concatenated sources the way the production build does,
        # pinned to the deterministic DOSBox runner (an auto-selected
        # MS-DOS Player backend can legitimately emit a few different bytes
        # for switch-table layouts in this group).
        plan = read_json(ROOT / 'layout/production-plan.json')['modules']
        group = next(m for m in plan if m['id'] == 'C_8A37_969D')
        owners = read_json(ROOT / 'recipes/c/matching-wave24.json')['owners']
        checked = sum(next(r for r in manifest['regions'] if r['id'] == o['id'])['end']
                     - next(r for r in manifest['regions'] if r['id'] == o['id'])['start']
                     for o in owners)
        self.assertEqual(checked, 866)

        def compile_group(overrides=None):
            overrides = overrides or {}
            chunks = [overrides.get(s, (ROOT / s).read_bytes()) for s in group['sources']]
            with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
                work = Path(temporary)
                concat = work / f"{group['id']}.C"
                concat.write_bytes(b'\r\n'.join(chunks))
                compile_owner = {'id': group['id'], 'kind': 'MATCHING_C',
                                 'source': concat.relative_to(ROOT).as_posix(),
                                 'build': {'flags_append': group['build'].get('flags_append', '')}}
                receipts, _ = compile_sources(ROOT, [compile_owner], work / 'compiler', ROOT / 'toolchain',
                                              resolve_runner(lock), lock)
                return read_object((work / 'compiler' / receipts[group['id']]['object']).read_bytes())

        module = compile_group()
        for owner_id in ('F_90A6', 'F_969D'):
            owner = next(r for r in manifest['regions'] if r['id'] == owner_id)
            data, _ = bind_region(owner, module, MZ.parse(original), manifest['frames'], manifest['regions'], modules)
            mismatch(original[owner['start']:owner['end']], data, owner)

        for name, before, after in (
            ('F_90A6', b'((int *)ui_gfx_shadow_a+2)[k]', b'((int *)ui_gfx_shadow_a)[k+2]'),
            # F_969D was normalized to the shared GC316.H record's 'rot'
            # field name instead of the old raw struct's 'b' field; the
            # rotation-3 comparison constant it mutates is unchanged.
            ('F_969D', b'puzzle_grid[i][j].rot!=3', b'puzzle_grid[i][j].rot!=2')):
            owner = next(o for o in owners if o['id'] == name)
            source = (ROOT / owner['source']).read_bytes()
            self.assertEqual(source.count(before), 1)
            mutant_module = compile_group(overrides={owner['source']: source.replace(before, after)})
            region = next(r for r in manifest['regions'] if r['id'] == name)
            with self.assertRaises(ValueError):
                data, _ = bind_region(region, mutant_module, MZ.parse(original), manifest['frames'],
                                      manifest['regions'], modules)
                mismatch(original[region['start']:region['end']], data, region)


if __name__ == '__main__':
    unittest.main()
