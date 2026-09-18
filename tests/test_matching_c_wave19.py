"""Full fresh comparisons and expression/layout mutants for the nineteenth C wave."""
import copy
from pathlib import Path
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from mz import MZ
from reconstruct import (read_json, compile_sources, read_object, bind_region, mismatch,
                         owned_library_modules, component_binding, library_candidate)


class MatchingCWave19Tests(unittest.TestCase):
    def test_fresh_wave_and_expression_layout_mutants(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        owners = [o for o in read_json(ROOT / 'recipes/c/matching-wave19.json')['owners'] if o['kind'] == 'MATCHING_C']
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        lock = read_json(ROOT / 'layout/toolchain.json')
        modules = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            work = Path(temporary)
            mutants = []
            for name, before, after in (
                ('F_A525', b'switch(c=fa43c(x+1,y))', b'c=fa43c(x+1,y);switch(c)'),):
                owner = copy.deepcopy(next(o for o in owners if o['id'] == name))
                source = (ROOT / owner['source']).read_bytes()
                self.assertEqual(source.count(before), 1)
                path = work / (name + '.C')
                path.write_bytes(source.replace(before, after))
                owner.update(id=name + '_MUTANT', source=path.relative_to(ROOT).as_posix())
                mutants.append(owner)
            receipts, _ = compile_sources(ROOT, owners + mutants, work / 'compiler', ROOT / 'toolchain',
                                          Path(lock['dosbox_default']), lock)
            checked = 0
            for owner in owners:
                module = read_object((work / 'compiler' / receipts[owner['id']]['object']).read_bytes())
                data, _ = bind_region(owner, module, MZ.parse(original), manifest['frames'], manifest['regions'], modules)
                mismatch(original[owner['start']:owner['end']], data, owner)
                checked += len(data)
            self.assertEqual(checked, 307)
            for owner in mutants:
                module = read_object((work / 'compiler' / receipts[owner['id']]['object']).read_bytes())
                with self.assertRaises(ValueError):
                    data, _ = bind_region(owner, module, MZ.parse(original), manifest['frames'], manifest['regions'], modules)
                    mismatch(original[owner['start']:owner['end']], data, owner)

    def test_complete_library_data_and_public_binding(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        lock = read_json(ROOT / 'layout/toolchain.json')
        owners = manifest['regions']
        modules = owned_library_modules(owners, ROOT / 'toolchain', lock)
        owner = next(o for o in owners if o['id'] == 'LIB_CTYPE_DATA')
        caller = next(o for o in owners if o['id'] == 'F_A525')
        binding = caller['build']['bindings']['_g37cb']
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        mz = MZ.parse(original)
        module = modules[owner['id']]
        data, _ = bind_region(owner, module, mz, manifest['frames'], owners, modules)
        mismatch(original[owner['start']:owner['end']], data, owner)
        self.assertEqual(len(data), 257)
        self.assertEqual(component_binding(binding, owners, mz, manifest['frames'], modules)['offset'], 0x37cb)
        for change in ({'public': '__missing'}, {'addend': 257}, {'addend': -1}, {'coordinate': 'code_offset'}):
            with self.assertRaises(ValueError):
                component_binding(dict(binding, **change), owners, mz, manifest['frames'], modules)
        with self.assertRaises(ValueError):
            component_binding(binding, owners, mz, manifest['frames'], {})
        shifted = copy.deepcopy(module)
        shifted.publics[0]['offset'] = 256
        with self.assertRaises(ValueError):
            component_binding(binding, owners, mz, manifest['frames'], {owner['id']: shifted})
        with self.assertRaises(ValueError):
            bind_region(dict(owner, end=owner['end']-1), module, mz, manifest['frames'], owners, modules)
        changed = copy.deepcopy(module)
        altered = bytearray(data)
        altered[66] ^= 1
        changed.segments['_DATA'] = bytes(altered)
        bad, _ = bind_region(owner, changed, mz, manifest['frames'], owners, modules)
        with self.assertRaises(ValueError):
            mismatch(original[owner['start']:owner['end']], bad, owner)
        with self.assertRaises(ValueError):
            library_candidate(owner, {'CTYPE': b'changed library module'})
