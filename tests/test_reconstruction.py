"""Negative controls for ownership, fixups, source derivation and file identity."""
import copy
import json
from pathlib import Path
import struct
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from mz import MZ
from omf import ObjectModule
from reconstruct import (bind_region, compile_sources, mismatch, read_object,
                         reconstruct, validate_layout)


class ReconstructionTests(unittest.TestCase):
    def setUp(self):
        self.manifest = json.loads((ROOT / 'layout/manifest.json').read_text())
        self.original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        self.mz = MZ.parse(self.original)

    def test_partition_rejects_gaps_overlaps_zero_and_wrong_total(self):
        for delta, error in ((1, 'gap'), (-1, 'overlap')):
            manifest = copy.deepcopy(self.manifest)
            manifest['regions'][1]['start'] += delta
            with self.assertRaisesRegex(ValueError, error):
                validate_layout(manifest)
        manifest = copy.deepcopy(self.manifest)
        manifest['regions'][0]['end'] = 0
        with self.assertRaisesRegex(ValueError, 'invalid range size'):
            validate_layout(manifest)
        manifest = copy.deepcopy(self.manifest)
        manifest['original']['size'] += 1
        with self.assertRaisesRegex(ValueError, 'gap'):
            validate_layout(manifest)

    def test_raw_corruption_and_truncation_identify_owner(self):
        owner = self.manifest['regions'][0]
        raw = (ROOT / owner['source']).read_bytes()
        corrupt = bytearray(raw)
        corrupt[40] ^= 1
        with self.assertRaisesRegex(ValueError, 'file 0x000028, owner RAW_000000'):
            mismatch(raw, corrupt, owner)
        with self.assertRaisesRegex(ValueError, 'wrong output length'):
            mismatch(raw, raw[:-1], owner)

    def test_mz_coordinates_relocations_and_invalid_header(self):
        self.assertEqual(self.mz.file_offset(0x56C6), 0x58C6)
        self.assertEqual(self.mz.load_offset(0x58C6), 0x56C6)
        self.assertEqual(len(self.mz.relocations), 106)
        with self.assertRaises(ValueError):
            self.mz.load_offset(10)
        corrupt = bytearray(self.original)
        struct.pack_into('<H', corrupt, 24, 510)
        with self.assertRaisesRegex(ValueError, 'relocation table'):
            MZ.parse(corrupt)

    def test_omf_truncation_and_checksum(self):
        with self.assertRaisesRegex(ValueError, 'Truncated'):
            read_object(b'\x80')
        with self.assertRaisesRegex(ValueError, 'checksum'):
            read_object(b'\x80\x02\x00\x00\x01')
        with self.assertRaisesRegex(ValueError, 'missing MODEND'):
            read_object(b'')

    def test_fixups_addends_self_relative_and_relocation_obligations(self):
        owner = {'id': 'TEST', 'start': 512 + 0x100, 'end': 512 + 0x108,
                 'build': {'segment': '_TEXT', 'public': '_test', 'bindings': {
                     '_near': {'offset': 0x200, 'coordinate': 'code_offset'},
                     '_far': {'offset': 0x300, 'coordinate': 'code_offset'}}, 'module_segments': {}}}
        fixes = [dict(segment='_TEXT', offset=0, width=2, loc='offset16', self_relative=True,
                      target_kind='external', target='_near', displacement=3),
                 dict(segment='_TEXT', offset=2, width=4, loc='pointer32', self_relative=False,
                      target_kind='external', target='_far', displacement=4),
                 dict(segment='_TEXT', offset=6, width=2, loc='base16', self_relative=False,
                      target_kind='group', target='DGROUP', displacement=0)]
        module = ObjectModule({'_TEXT': struct.pack('<4H', 2, 5, 0, 0)},
                              [{'name': '_test', 'segment': '_TEXT', 'offset': 0}], fixes, [],
                              segment_lengths={'_TEXT': 8})
        mz = copy.deepcopy(self.mz)
        mz.relocations = [{'load_offset': 0x104}, {'load_offset': 0x106}]
        result, _ = bind_region(owner, module, mz, self.manifest['frames'])
        self.assertEqual(struct.unpack('<4H', result), (0x103, 0x309, 0, 0xFA3))
        mz.relocations.pop()
        with self.assertRaisesRegex(ValueError, 'relocation map differs'):
            bind_region(owner, module, mz, self.manifest['frames'])
        owner['build']['bindings'].pop('_near')
        with self.assertRaisesRegex(ValueError, 'unresolved'):
            bind_region(owner, module, mz, self.manifest['frames'])

    def test_failure_invalidates_old_success(self):
        with tempfile.TemporaryDirectory(dir=ROOT / 'build', prefix='test-') as tmp:
            directory = Path(tmp)
            (directory / 'AEPROG.EXE').write_bytes(b'stale')
            (directory / 'report.json').write_text('{}')
            bad = copy.deepcopy(self.manifest)
            bad['regions'][1]['start'] += 1
            manifest_path = directory / 'manifest.json'
            manifest_path.write_text(json.dumps(bad))
            with self.assertRaisesRegex(ValueError, 'gap'):
                reconstruct(ROOT, manifest_path, directory, ROOT / 'toolchain', Path('unused'))
            self.assertFalse((directory / 'AEPROG.EXE').exists())
            self.assertFalse((directory / 'report.json').exists())

    def test_fresh_c_and_asm_mutants_and_binding_mutant_are_rejected(self):
        lock = json.loads((ROOT / 'layout/toolchain.json').read_text())
        dosbox = Path(lock['dosbox_default'])
        with tempfile.TemporaryDirectory(dir=ROOT / 'build', prefix='test-') as tmp:
            directory = Path(tmp)
            owners = []
            for name, before, after in [('F_56C6', b'g96 = 0x18f;', b'g96 = 0x18e;'),
                                         ('F_D89A', b'        cld', b'        std')]:
                owner = copy.deepcopy(next(r for r in self.manifest['regions'] if r['id'] == name))
                source = (ROOT / owner['source']).read_bytes()
                self.assertIn(before, source)
                path = directory / owner['source']
                path.parent.mkdir(exist_ok=True)
                path.write_bytes(source)
                owners.append(owner)
                mutant = copy.deepcopy(owner)
                mutant['id'] += '_MUTANT'
                mutant['source'] = 'mutant_' + Path(owner['source']).name
                (directory / mutant['source']).write_bytes(source.replace(before, after, 1))
                owners.append(mutant)
            work = directory / 'session'
            work.mkdir()
            receipts, _ = compile_sources(directory, owners, work, ROOT / 'toolchain', dosbox, lock)
            for owner in owners:
                module = read_object((work / receipts[owner['id']]['object']).read_bytes())
                result, _ = bind_region(owner, module, self.mz, self.manifest['frames'])
                expected = self.original[owner['start']:owner['end']]
                if owner['id'].endswith('_MUTANT'):
                    with self.assertRaisesRegex(ValueError, 'First mismatch.*' + owner['id']):
                        mismatch(expected, result, owner)
                else:
                    mismatch(expected, result, owner)
            owner = owners[0]
            owner['build']['bindings']['_mode']['offset'] += 1
            module = read_object((work / receipts[owner['id']]['object']).read_bytes())
            result, _ = bind_region(owner, module, self.mz, self.manifest['frames'])
            with self.assertRaisesRegex(ValueError, 'First mismatch.*F_56C6'):
                mismatch(self.original[owner['start']:owner['end']], result, owner)


if __name__ == '__main__':
    (ROOT / 'build').mkdir(exist_ok=True)
    unittest.main()
