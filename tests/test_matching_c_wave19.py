"""Full fresh comparisons and expression/layout mutants for the nineteenth C wave.

The fresh-wave/mutant part below is converted to use the canonical prover
(tools/probe_module.py) via tests/support_probe.py; the library data and
public binding test asserts different artifacts and is left unchanged.
"""
import copy
from pathlib import Path
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
sys.path.insert(0, str(Path(__file__).resolve().parent))
from mz import MZ
from reconstruct import (read_json, bind_region, mismatch,
                         owned_library_modules, component_binding, library_candidate)
from support_probe import exact, mutant_rejected


class MatchingCWave19Tests(unittest.TestCase):
    def test_fresh_wave_and_expression_layout_mutants(self):
        owners = [o for o in read_json(ROOT / 'recipes/c/matching-wave19.json')['owners'] if o['kind'] == 'MATCHING_C']
        for owner in owners:
            exact(owner['id'])
        for name, before, after in (
            ('F_A525', b'switch (c = slot_input_wait_key(x + 1, y)) {',
             b'c = slot_input_wait_key(x + 1, y); switch (c) {'),):
            mutant_rejected(name, before, after)

    def test_complete_library_data_and_public_binding(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        lock = read_json(ROOT / 'layout/toolchain.json')
        owners = manifest['regions']
        modules = owned_library_modules(owners, ROOT / 'toolchain', lock)
        owner = next(o for o in owners if o['id'] == 'LIB_CTYPE_DATA')
        caller = next(o for o in owners if o['id'] == 'F_A525')
        binding = caller['build']['bindings']['__ctype']
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        mz = MZ.parse(original)
        module = modules[owner['id']]
        data, _ = bind_region(owner, module, mz, manifest['frames'], owners, modules)
        mismatch(original[owner['start']:owner['end']], data, owner)
        self.assertEqual(len(data), 257)
        self.assertEqual(component_binding(binding, owners, mz, manifest['frames'], modules)['offset'], 0x37ca)
        for change in ({'public': '__missing'}, {'addend': 257}, {'addend': -1}, {'coordinate': 'code_offset'}):
            with self.assertRaises(ValueError):
                component_binding(dict(binding, **change), owners, mz, manifest['frames'], modules)
        with self.assertRaises(ValueError):
            component_binding(binding, owners, mz, manifest['frames'], {})
        shifted = copy.deepcopy(module)
        shifted.publics[0]['offset'] = 257
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
