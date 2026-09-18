"""Identified data round trips, edited source, and component address controls."""
import copy
import json
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from exe_data import DAC_FORMAT, palette_document, encode_data
from mz import MZ
from reconstruct import component_binding, reconstruct


class ExeDataTests(unittest.TestCase):
    def setUp(self):
        self.manifest = json.loads((ROOT / 'layout/manifest.json').read_text())
        self.original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        self.owners = [r for r in self.manifest['regions'] if r.get('classification') == 'embedded_palette']

    def test_real_palettes_and_editable_channels(self):
        self.assertEqual(len(self.owners), 2)
        for owner in self.owners:
            original = self.original[owner['start']:owner['end']]
            source = json.loads((ROOT / owner['source']).read_text())
            self.assertEqual(palette_document(original), source)
            self.assertEqual(encode_data(source, DAC_FORMAT), original)
            source['entries'][0][0] = (source['entries'][0][0] + 1) % 64
            changed = encode_data(source, DAC_FORMAT)
            self.assertNotEqual(changed[0], original[0])
            self.assertEqual(changed[1:], original[1:])

    def test_invalid_channels_shape_and_fields(self):
        for value in (-1, 64, True, 1.5, '0'):
            source = palette_document(bytes(768))
            source['entries'][0][0] = value
            with self.assertRaises(ValueError):
                encode_data(source, DAC_FORMAT)
        for data in (bytes(767), bytes([64]) * 768):
            with self.assertRaises(ValueError):
                palette_document(data)
        for mutate in (lambda d: d['entries'].pop(), lambda d: d.update(extra=0),
                       lambda d: d['entries'][0].pop()):
            source = palette_document(bytes(768))
            mutate(source)
            with self.assertRaises(ValueError):
                encode_data(source, DAC_FORMAT)

    def test_owner_address_moves_with_component(self):
        owner = copy.deepcopy(self.owners[0])
        binding = {'owner': owner['id'], 'coordinate': 'DGROUP_offset', 'addend': 0}
        mz, frames = MZ.parse(self.original), self.manifest['frames']
        self.assertEqual(component_binding(binding, [owner], mz, frames)['offset'], 0x11e)
        owner['start'] += 16
        owner['end'] += 16
        self.assertEqual(component_binding(binding, [owner], mz, frames)['offset'], 0x12e)
        for changes in ({'owner': 'missing'}, {'offset': 286}, {'addend': 768},
                        {'addend': -1}, {'addend': True}, {'coordinate': 'code_offset'}):
            with self.assertRaises(ValueError):
                component_binding({**binding, **changes}, [owner], mz, frames)

    def test_changed_palette_rejected_before_compiler(self):
        import reconstruct as implementation
        real_read = implementation.read_json
        def changed_read(path):
            result = real_read(path)
            if path.name == Path(self.owners[0]['source']).name:
                result['entries'][0][0] = (result['entries'][0][0] + 1) % 64
            return result
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            with patch('reconstruct.read_json', side_effect=changed_read), patch('reconstruct.compile_sources') as compile_mock:
                with self.assertRaisesRegex(ValueError, 'First mismatch.*PALETTE'):
                    reconstruct(ROOT, ROOT / 'layout/manifest.json', Path(temporary), ROOT / 'toolchain', Path('unused'))
                compile_mock.assert_not_called()


if __name__ == '__main__':
    unittest.main()
