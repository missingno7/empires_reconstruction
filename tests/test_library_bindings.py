"""Library public resolution derives relative locations from pinned OMF data."""
import copy
import json
from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from mz import MZ
from reconstruct import owned_library_modules, component_binding
from recover_library_bindings import derive


class LibraryBindingTests(unittest.TestCase):
    def setUp(self):
        self.manifest = json.loads((ROOT / 'layout/manifest.json').read_text())
        self.mz = MZ.parse((ROOT / 'assets/AEPROG.EXE').read_bytes())
        self.modules = owned_library_modules(self.manifest['regions'], ROOT / 'toolchain',
                                            json.loads((ROOT / 'layout/toolchain.json').read_text()))
        self.owner = copy.deepcopy(next(r for r in self.manifest['regions'] if r['id'] == 'LIB_FMALLOC'))
        self.binding = {'owner': 'LIB_FMALLOC', 'public': '_farmalloc', 'coordinate': 'code_offset', 'addend': 0}

    def resolve(self):
        return component_binding(self.binding, [self.owner], self.mz, self.manifest['frames'], self.modules)['offset']

    def test_nonzero_public_and_component_movement(self):
        initial = self.resolve()
        self.assertEqual(initial, self.mz.load_offset(self.owner['start']) + 507)
        self.owner['start'] += 16
        self.owner['end'] += 16
        self.assertEqual(self.resolve(), initial + 16)
        public = next(p for p in self.modules['LIB_FMALLOC'].publics if p['name'] == '_farmalloc')
        public['offset'] += 1
        self.assertEqual(self.resolve(), initial + 17)

    def test_missing_duplicate_and_outside_public_rejected(self):
        original = copy.deepcopy(self.modules['LIB_FMALLOC'])
        with self.assertRaisesRegex(ValueError, 'verified component module'):
            component_binding(self.binding, [self.owner], self.mz, self.manifest['frames'])
        for variant in ('missing', 'duplicate', 'outside'):
            module = copy.deepcopy(original)
            public = next(p for p in module.publics if p['name'] == '_farmalloc')
            if variant == 'missing':
                module.publics.remove(public)
            elif variant == 'duplicate':
                module.publics.append(copy.deepcopy(public))
            else:
                public['offset'] = self.owner['end'] - self.owner['start']
            self.modules['LIB_FMALLOC'] = module
            with self.assertRaises(ValueError):
                self.resolve()

    def test_recorded_migration_and_idempotence(self):
        receipt = json.loads((ROOT / 'docs/library-binding-evidence.json').read_text())
        restored = copy.deepcopy(self.manifest)
        owners = {r['id']: r for r in restored['regions']}
        for change in receipt['changes']:
            binding = owners[change['caller']]['build']['bindings'][change['symbol']]
            resolved = component_binding(binding, restored['regions'], self.mz, self.manifest['frames'], self.modules)
            self.assertEqual(resolved['offset'], change['previous_code_offset'])
            for key in ('owner', 'public', 'addend'):
                binding.pop(key)
            binding['offset'] = change['previous_code_offset']
        updated, changes = derive(restored, self.mz, self.modules)
        self.assertEqual(updated, self.manifest)
        self.assertEqual(len(changes), 38)
        self.assertEqual(derive(updated, self.mz, self.modules), (updated, []))

    def test_ambiguous_public_is_not_promoted(self):
        manifest = copy.deepcopy(self.manifest)
        caller = next(r for r in manifest['regions'] if r['id'] == 'F_0281')
        caller['build']['bindings']['_farmalloc'] = {'coordinate': 'code_offset', 'offset': self.resolve()}
        module = self.modules['LIB_FMALLOC']
        module.publics.append(copy.deepcopy(next(p for p in module.publics if p['name'] == '_farmalloc')))
        updated, changes = derive(manifest, self.mz, self.modules)
        self.assertEqual(updated, manifest)
        self.assertEqual(changes, [])
