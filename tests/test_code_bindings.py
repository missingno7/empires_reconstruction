"""Owned entry references, exact migration and near/far relocation controls."""
import copy
import json
from pathlib import Path
import struct
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from mz import MZ
from omf import ObjectModule
from reconstruct import component_binding, bind_region
from recover_code_bindings import derive


class CodeBindingTests(unittest.TestCase):
    def setUp(self):
        self.manifest = json.loads((ROOT / 'layout/manifest.json').read_text())
        self.mz = MZ.parse((ROOT / 'assets/AEPROG.EXE').read_bytes())
        self.frames = self.manifest['frames']

    def test_migration_matches_recorded_addresses_and_is_idempotent(self):
        receipt = json.loads((ROOT / 'docs/code-binding-evidence.json').read_text())
        owners = {r['id']: r for r in self.manifest['regions']}
        restored = copy.deepcopy(self.manifest)
        old_owners = {r['id']: r for r in restored['regions']}
        for change in receipt['changes']:
            binding = owners[change['caller']]['build']['bindings'][change['symbol']]
            resolved = component_binding(binding, list(owners.values()), self.mz, self.frames)
            self.assertEqual(resolved['offset'], change['previous_code_offset'])
            old = old_owners[change['caller']]['build']['bindings'][change['symbol']]
            for key in ('owner', 'public', 'addend'):
                old.pop(key)
            old['offset'] = change['previous_code_offset']
        updated, changes = derive(restored, self.mz)
        self.assertEqual(updated, self.manifest)
        self.assertEqual(len(changes), 151)
        self.assertEqual(derive(updated, self.mz), (updated, []))

    def test_code_owner_validation_and_placement(self):
        owner = copy.deepcopy(next(r for r in self.manifest['regions'] if r['id'] == 'F_01BC'))
        binding = {'owner': owner['id'], 'public': owner['build']['public'], 'addend': 0, 'coordinate': 'code_offset'}
        self.assertEqual(component_binding(binding, [owner], self.mz, self.frames)['offset'], 0x1bc)
        owner['start'] += 16
        owner['end'] += 16
        self.assertEqual(component_binding(binding, [owner], self.mz, self.frames)['offset'], 0x1cc)
        for change in ({'public': '_wrong'}, {'addend': 1}, {'coordinate': 'DGROUP_offset'}, {'offset': 0}):
            with self.assertRaises(ValueError):
                component_binding({**binding, **change}, [owner], self.mz, self.frames)
        owner['kind'] = 'RAW'
        with self.assertRaises(ValueError):
            component_binding(binding, [owner], self.mz, self.frames)

    def test_owned_near_and_far_fixups_move_with_target(self):
        target = {'id': 'TARGET', 'kind': 'MATCHING_C', 'start': 512 + 0x300, 'end': 512 + 0x310,
                  'build': {'public': '_target', 'segment': '_TEXT'}}
        owner = {'id': 'CALLER', 'start': 512 + 0x100, 'end': 512 + 0x106,
                 'build': {'public': '_caller', 'segment': '_TEXT', 'module_segments': {},
                           'bindings': {'_target': {'owner': 'TARGET', 'public': '_target',
                                                   'coordinate': 'code_offset', 'addend': 0}}}}
        fixes = [dict(segment='_TEXT', offset=0, width=2, loc='offset16', self_relative=True,
                      target_kind='external', target='_target', displacement=3),
                 dict(segment='_TEXT', offset=2, width=4, loc='pointer32', self_relative=False,
                      target_kind='external', target='_target', displacement=4)]
        module = ObjectModule({'_TEXT': struct.pack('<3H', 2, 5, 0)},
                              [{'name': '_caller', 'segment': '_TEXT', 'offset': 0}], fixes, ['_target'],
                              segment_lengths={'_TEXT': 6})
        mz = copy.deepcopy(self.mz)
        mz.relocations = [{'load_offset': 0x104}]
        for movement in (0, 16):
            target['start'] = 512 + 0x300 + movement
            target['end'] = target['start'] + 16
            result, _ = bind_region(owner, module, mz, self.frames, [target])
            self.assertEqual(struct.unpack('<3H', result), (0x203 + movement, 0x309 + movement, 0))
        mz.relocations = []
        with self.assertRaisesRegex(ValueError, 'relocation map differs'):
            bind_region(owner, module, mz, self.frames, [target])
