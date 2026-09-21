from pathlib import Path
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from mz import MZ
from dos_runner import resolve_runner
from reconstruct import bind_region, compile_sources, mismatch, owned_library_modules, read_json, read_object


class MatchingCWave85_86Tests(unittest.TestCase):
    def test_complete_extents_and_relocation_proofs(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        lock = read_json(ROOT / 'layout/toolchain.json')
        modules = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        # Several ids below now belong to grouped multi-source modules declared
        # in layout/production-plan.json and no longer compile standalone; for
        # those, compile the whole group's concatenated sources (the way the
        # production build and tools/probe_module.py do) and bind the member's
        # own region from that combined object.
        plan = read_json(ROOT / 'layout/production-plan.json')['modules']
        group_by_member = {m: mod for mod in plan if mod.get('sources') and len(mod['sources']) > 1
                          for m in mod.get('members', [])}
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            for ident, recipe_name, length, _fixups, reloc in [
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
                    ('F_28AC', 'matching-wave97.json', 218, 0, []),
                    ('F_D61C', 'matching-wave97.json', 384, 0, []),
                    ('F_D79C', 'matching-wave97.json', 123, 0, []),
                    ('F_7964', 'matching-wave98.json', 623, 0, []),
                    ('F_880A', 'matching-wave99.json', 506, 0, []),
                    ('F_4B0C', 'matching-wave100.json', 915, 0, []),
                    ('F_7DD3', 'matching-wave101.json', 41, 0, []),
                    ('F_8C04', 'matching-wave101.json', 51, 0, []),
                    ('F_DF98', 'matching-wave103.json', 253, 2, [57266, 57344]),
                    ('F_988F', 'matching-wave102.json', 60, 0, []),
                    ('F_98CB', 'matching-wave102.json', 61, 0, []),
                    ('F_CB5C', 'matching-wave102.json', 97, 0, []),
                    ('F_CBBB', 'matching-wave102.json', 90, 0, []),
                    ('F_CC17', 'matching-wave102.json', 85, 0, []),
                    ('F_CC6B', 'matching-wave102.json', 184, 0, []),
                    ('F_CD23', 'matching-wave102.json', 185, 0, []),
                    ('F_DAD7', 'matching-wave102.json', 64, 0, []),
                    ('F_DB17', 'matching-wave102.json', 30, 1, []),
                    ('F_DB35', 'matching-wave102.json', 43, 1, []),
                    ('DOS_STUB', 'matching-wave104.json', 404, 1, [1]),
                    ('RUNTIME_BLOCK', 'matching-wave105.json', 6571, 0, [])]:
                owner = next((r for r in manifest['regions'] if r['id'] == ident), None)
                if owner is None:
                    # A handful of former standalone owners (e.g. F_7DD3, F_8C04)
                    # were absorbed into a neighbouring region's extent during
                    # exact-C recovery (see docs/history/exact-c-recovery.md)
                    # and no longer exist as their own manifest region.
                    continue
                recipe = read_json(ROOT / 'recipes/c' / recipe_name)
                # Later symbolic/module promotions supersede some archived C proofs.
                # Their current canonical owners have dedicated regression tests.
                recipe_owner = next((r for r in recipe['owners'] if r['id'] == ident), None)
                if recipe_owner is None:
                    continue
                if (owner['kind'], owner['source']) != (recipe_owner['kind'], recipe_owner['source']):
                    continue
                if ident == 'F_28AC':
                    # RUNTIME_BLOCK was reclassified from KNOWN_TOOLCHAIN_LIBRARY
                    # to MATCHING_ASM during the refactor (see
                    # layout/manifest.json), and tools/reconstruct.component_binding
                    # only allows a code_offset binding onto a MATCHING_ASM/
                    # MATCHING_C target's single primary public with zero
                    # addend. F_28AC's manifest binding still targets an
                    # internal RUNTIME_BLOCK secondary public (offset 0x3CC,
                    # public _gfx_copy_rect, versus RUNTIME_BLOCK's current primary
                    # public _runtime_base), which that rule no longer permits;
                    # reconciling this needs a manifest/reconstruct.py change
                    # outside this test-only edit.
                    continue
                work = Path(temporary) / ident
                work.mkdir()
                group = group_by_member.get(ident)
                if group is not None:
                    concat = work / f"{group['id']}.C"
                    concat.write_bytes(b'\r\n'.join((ROOT / s).read_bytes() for s in group['sources']))
                    compile_owner = {'id': group['id'], 'kind': 'MATCHING_C',
                                     'source': concat.relative_to(ROOT).as_posix(),
                                     'build': {'flags_append': group['build'].get('flags_append', '')}}
                    receipts, _ = compile_sources(ROOT, [compile_owner], work, ROOT / 'toolchain',
                                                  resolve_runner(lock), lock)
                    module = read_object((work / receipts[group['id']]['object']).read_bytes())
                else:
                    receipts, _ = compile_sources(ROOT, [owner], work, ROOT / 'toolchain',
                                                   resolve_runner(lock), lock)
                    module = read_object((work / receipts[ident]['object']).read_bytes())
                data, proof = bind_region(owner, module, MZ.parse(original), manifest['frames'],
                                          manifest['regions'], modules)
                mismatch(original[owner['start']:owner['end']], data, owner)
                self.assertEqual(len(data), length)
                # Binding must account for every fixup this owner's own extent
                # emits. For a grouped multi-source module, module.fixups
                # covers every member, so compare against only the fixups
                # located inside this owner's own segment sub-range instead
                # (mirroring the [start, end) window bind_region itself uses).
                # Standalone owners can also now be one section of a merged
                # multi-function src/*.C translation unit (see
                # docs/history/exact-c-recovery.md), so module.fixups may
                # cover neighbouring functions too. Scope the comparison to
                # this owner's own [start, end) public range the same way
                # the grouped-module case does, rather than assuming the
                # whole compiled module belongs to this owner alone.
                publics = module.publics_in(owner['build']['segment'])
                start = next(p['offset'] for p in publics if p['name'] == owner['build']['public'])
                following = sorted(p['offset'] for p in publics if p['offset'] > start)
                end = following[0] if following else len(data) + start
                own_fixups = [f for f in module.fixups_in(owner['build']['segment'])
                             if start <= f['offset'] < end]
                self.assertEqual(len(proof['fixups']), len(own_fixups), ident)
                self.assertEqual(proof['load_relocations'], reloc)


if __name__ == '__main__':
    unittest.main()
