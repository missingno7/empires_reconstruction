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
        cls.owners = read_json(ROOT / 'recipes/c/matching-wave3.json')['owners']
        cls.original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        cls.mz = MZ.parse(cls.original)
        cls.lock = read_json(ROOT / 'layout/toolchain.json')
        cls.modules = owned_library_modules(cls.manifest['regions'], ROOT / 'toolchain', cls.lock)
        cls.temporary = tempfile.TemporaryDirectory(dir=ROOT / 'build')
        cls.addClassCleanup(cls.temporary.cleanup)
        work = Path(cls.temporary.name)
        sources = [o for o in cls.owners if o['kind'] == 'MATCHING_C']
        owner = copy.deepcopy(next(o for o in sources if o['id'] == 'F_75F3'))
        original_source = (ROOT / owner['source']).read_bytes()
        assert original_source.count(b'to Continue') == 1
        path = work / 'mutant.C'
        path.write_bytes(original_source.replace(b'to Continue', b'to continue'))
        owner.update(id='CHANGED_INITIALIZER', source=path.relative_to(ROOT).as_posix())
        receipts, _ = compile_sources(ROOT, sources + [owner], work / 'compiler', ROOT / 'toolchain',
                                      Path(cls.lock['dosbox_default']), cls.lock)
        for name, receipt in receipts.items():
            cls.modules[name] = read_object((work / 'compiler' / receipt['object']).read_bytes())

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
        self.assertEqual(counts, {'MATCHING_C': 821, 'EXACT_DATA': 15})

    def test_initializer_edit_changes_emitted_data_even_when_code_stays_equal(self):
        code = next(o for o in self.owners if o['id'] == 'F_75F3')
        owner = next(o for o in self.owners if o['id'] == 'C_DATA_75F3')
        modules = {**self.modules, 'F_75F3': self.modules['CHANGED_INITIALIZER']}
        data, _ = bind_region(code, modules['F_75F3'], self.mz, self.manifest['frames'], self.manifest['regions'], modules)
        mismatch(self.original[code['start']:code['end']], data, code)
        changed, _ = compiled_data(owner, self.manifest['regions'], modules, self.mz)
        with self.assertRaisesRegex(ValueError, 'First mismatch.*C_DATA_75F3'):
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
