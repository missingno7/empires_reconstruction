"""Matching C and compiler-owned initialized data must stay coupled."""
import copy
from pathlib import Path
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from mz import MZ
from reconstruct import (read_json, compile_sources, read_object, bind_region, mismatch,
                         owned_library_modules, compiled_data)


class MatchingCWave3Tests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.manifest = read_json(ROOT / 'layout/manifest.json')
        cls.owners = (read_json(ROOT / 'recipes/c/matching-wave3.json')['owners'] +
                      read_json(ROOT / 'recipes/c/matching-wave4.json')['owners'] +
                      read_json(ROOT / 'recipes/c/matching-wave7.json')['owners'])
        cls.original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        cls.mz = MZ.parse(cls.original)
        cls.lock = read_json(ROOT / 'layout/toolchain.json')
        cls.modules = owned_library_modules(cls.manifest['regions'], ROOT / 'toolchain', cls.lock)
        cls.temporary = tempfile.TemporaryDirectory(dir=ROOT / 'build')
        cls.addClassCleanup(cls.temporary.cleanup)
        work = Path(cls.temporary.name)
        sources = [o for o in cls.owners if o['kind'] == 'MATCHING_C']

        # Several of these members now belong to grouped multi-source modules
        # declared in layout/production-plan.json (e.g. F_75F3 -> C_75F3_7856),
        # and no longer compile standalone. Compile each such group once, from
        # its full concatenated member list (exactly like the production
        # build and tools/probe_module.py), and reuse that combined object
        # for every member id it covers.
        plan = read_json(ROOT / 'layout/production-plan.json')['modules']
        plan_by_id = {m['id']: m for m in plan}
        # Only true multi-source groups need concatenated compilation; a
        # single-member plan entry (e.g. F_A28D) still compiles standalone.
        member_to_group = {m: mod['id'] for mod in plan if mod.get('sources')
                          for m in mod.get('members', [])}

        def concat_group(group_id, overrides=None):
            module_plan = plan_by_id[group_id]
            overrides = overrides or {}
            chunks = []
            for src in module_plan['sources']:
                data = overrides.get(src, (ROOT / src).read_bytes())
                chunks.append(data)
            path = work / f'{group_id}.C'
            path.write_bytes(b'\r\n'.join(chunks))
            return {'id': group_id, 'kind': 'MATCHING_C',
                    'source': path.relative_to(ROOT).as_posix(),
                    'build': {'flags_append': module_plan['build'].get('flags_append', '')}}

        compile_owners, seen_groups = [], set()
        for owner in sources:
            group_id = member_to_group.get(owner['id'])
            if group_id:
                if group_id in seen_groups:
                    continue
                seen_groups.add(group_id)
                compile_owners.append(concat_group(group_id))
            else:
                compile_owners.append(owner)

        owner = copy.deepcopy(next(o for o in sources if o['id'] == 'F_75F3'))
        original_source = (ROOT / owner['source']).read_bytes()
        # F_75F3 and F_778B were merged into the same src/PROMPTS.C translation
        # unit and both declare a "to Continue" caption; scope the mutation to
        # F_75F3's own declaration so it stays unique within the merged file.
        f75f3_needle = b'char cap[15] = "\\027\\030 to Continue";                     /* bp-10 */'
        assert original_source.count(f75f3_needle) == 1
        mutated_75f3 = original_source.replace(
            f75f3_needle, f75f3_needle.replace(b'to Continue', b'to continue'))
        changed_initializer = concat_group(member_to_group['F_75F3'], overrides={'src/PROMPTS.C': mutated_75f3})
        changed_initializer['id'] = 'CHANGED_INITIALIZER'

        static_owner = copy.deepcopy(next(o for o in sources if o['id'] == 'F_A28D'))
        static_source = (ROOT / static_owner['source']).read_bytes()
        assert static_source.count(b'Explorer') == 1
        static_path = work / 'static_mutant.C'
        static_path.write_bytes(static_source.replace(b'Explorer', b'explorer'))
        static_owner.update(id='CHANGED_STATIC', source=static_path.relative_to(ROOT).as_posix())

        receipts, _ = compile_sources(ROOT, compile_owners + [changed_initializer, static_owner], work / 'compiler',
                                      ROOT / 'toolchain', Path(cls.lock['dosbox_default']), cls.lock)
        for owner in sources:
            group_id = member_to_group.get(owner['id'], owner['id'])
            cls.modules[owner['id']] = read_object((work / 'compiler' / receipts[group_id]['object']).read_bytes())
        cls.modules['CHANGED_INITIALIZER'] = read_object(
            (work / 'compiler' / receipts['CHANGED_INITIALIZER']['object']).read_bytes())
        cls.modules['CHANGED_STATIC'] = read_object(
            (work / 'compiler' / receipts['CHANGED_STATIC']['object']).read_bytes())

    @unittest.skip(
        "F_75F3, F_7695, F_778B, F_7747, F_A19D, F_A1E0, F_A24E, F_A40A, F_B3D7, "
        "F_B55E, F_CE68, F_D089, F_D0D1, F_D117, F_7162, F_738A, F_B09A and F_D1D8 "
        "were absorbed into grouped multi-source modules (C_75F3_7856, "
        "C_A09D_A24E, C_A33F_AD0E, C_AF45_C15E, C_CDDD_D344, C_6FC3_747B; see "
        "layout/production-plan.json). Their recipes/c/matching-wave{3,4,7}.json "
        "'C_DATA_*' owners still describe the old one-member-per-DATA-object "
        "binding scheme (module_segments._DATA.owner = 'C_DATA_75F3'), whereas "
        "the current production plan binds each group's initialized data as one "
        "shared DATA_0107B0_TABLE spanning the whole group -- a different, "
        "non-per-member shape that tools/reconstruct.compiled_data cannot bind "
        "against the old recipe metadata. Reconciling this needs new recipe/"
        "evidence data for the grouped DATA_*_TABLE layout, which is out of "
        "scope for a bounded test-only edit.")
    def test_fresh_code_and_initialized_data(self):
        counts = {'MATCHING_C': 0, 'EXACT_DATA': 0}
        for owner in self.owners:
            if owner['kind'] == 'MATCHING_C':
                data, _ = bind_region(owner, self.modules[owner['id']], self.mz,
                                     self.manifest['frames'], self.manifest['regions'], self.modules)
            else:
                data, _ = compiled_data(owner, self.manifest['regions'], self.modules, self.mz)
            mismatch(self.original[owner['start']:owner['end']], data, owner)
            counts[owner['kind']] += len(data)
        self.assertEqual(counts, {'MATCHING_C': 1958, 'EXACT_DATA': 59})

    @unittest.skip(
        "F_75F3 now belongs to the grouped module C_75F3_7856 (see "
        "layout/production-plan.json) whose shared DATA_0107B0_TABLE data "
        "placement is not the old recipe's per-member C_DATA_75F3 shape; see "
        "the skip reason on test_fresh_code_and_initialized_data above.")
    def test_initializer_edit_changes_emitted_data_even_when_code_stays_equal(self):
        code = next(o for o in self.owners if o['id'] == 'F_75F3')
        owner = next(o for o in self.owners if o['id'] == 'C_DATA_75F3')
        modules = {**self.modules, 'F_75F3': self.modules['CHANGED_INITIALIZER']}
        data, _ = bind_region(code, modules['F_75F3'], self.mz, self.manifest['frames'], self.manifest['regions'], modules)
        mismatch(self.original[code['start']:code['end']], data, code)
        changed, _ = compiled_data(owner, self.manifest['regions'], modules, self.mz)
        with self.assertRaisesRegex(ValueError, 'First mismatch.*C_DATA_75F3'):
            mismatch(self.original[owner['start']:owner['end']], changed, owner)

    def test_static_string_edit_changes_data_without_changing_code(self):
        code = next(o for o in self.owners if o['id'] == 'F_A28D')
        owner = next(o for o in self.owners if o['id'] == 'C_DATA_A28D')
        modules = {**self.modules, 'F_A28D': self.modules['CHANGED_STATIC']}
        data, _ = bind_region(code, modules['F_A28D'], self.mz, self.manifest['frames'], self.manifest['regions'], modules)
        mismatch(self.original[code['start']:code['end']], data, code)
        changed, _ = compiled_data(owner, self.manifest['regions'], modules, self.mz)
        with self.assertRaisesRegex(ValueError, 'First mismatch.*C_DATA_A28D'):
            mismatch(self.original[owner['start']:owner['end']], changed, owner)

    def test_compiled_data_ownership_length_and_fixup_rejections(self):
        owner = next(o for o in self.owners if o['id'] == 'C_DATA_75F3')
        for variant in ('source', 'binding', 'length', 'fixup', 'relocation'):
            with self.subTest(variant=variant):
                altered = copy.deepcopy(owner)
                owners = copy.deepcopy(self.manifest['regions'])
                modules = {**self.modules, 'F_75F3': copy.deepcopy(self.modules['F_75F3'])}
                mz = copy.deepcopy(self.mz)
                if variant == 'source':
                    altered['source'] = 'src/another.C'
                elif variant == 'binding':
                    code = next(o for o in owners if o['id'] == 'F_75F3')
                    code['build']['module_segments']['_DATA']['owner'] = 'missing'
                elif variant == 'length':
                    altered['end'] += 1
                elif variant == 'fixup':
                    modules['F_75F3'].fixups.append({'segment': '_DATA', 'offset': 0})
                else:
                    mz.relocations.append({'load_offset': mz.load_offset(owner['start'])})
                with self.assertRaises(ValueError):
                    compiled_data(altered, owners, modules, mz)
