"""Lossless header encoding and rejection of structural or byte-level changes."""
import copy
import json
from pathlib import Path
import struct
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from mz import MZ, HEADER_FIELDS, encode_header, header_document
from reconstruct import mismatch, reconstruct, validate_layout


class MZHeaderTests(unittest.TestCase):
    def setUp(self):
        self.original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        self.document = header_document(self.original)
        self.manifest = json.loads((ROOT / 'layout/manifest.json').read_text())
        self.owner = self.manifest['regions'][0]

    def test_real_header_round_trip_and_preserved_gaps(self):
        source = json.loads((ROOT / 'layout/mz-header.json').read_text())
        self.assertEqual(source, self.document)
        encoded = encode_header(source, len(self.original))
        self.assertEqual(encoded, self.original[:512])
        self.assertEqual(encoded[28:34], bytes.fromhex('01 00 fb 20 72 6a'))
        self.assertEqual(encoded[458:], bytes(54))
        self.assertEqual(len(source['relocations']), 106)
        self.assertEqual(MZ.parse_header(encoded, len(self.original)), MZ.parse(self.original))
        self.assertEqual(tuple(source['fields']), HEADER_FIELDS)

    def test_reordered_and_aliased_relocations_are_not_normalized(self):
        changed = copy.deepcopy(self.document)
        changed['relocations'][0], changed['relocations'][1] = changed['relocations'][1], changed['relocations'][0]
        encoded = encode_header(changed, len(self.original))
        with self.assertRaisesRegex(ValueError, 'First mismatch.*MZ_HEADER'):
            mismatch(self.original[:512], encoded, self.owner)
        alias = copy.deepcopy(self.document)
        entry = alias['relocations'][1]
        entry['offset'] -= 16
        entry['segment'] += 1
        aliased = encode_header(alias, len(self.original))
        original_targets = [r['load_offset'] for r in MZ.parse(self.original).relocations]
        alias_targets = [r['load_offset'] for r in MZ.parse_header(aliased, len(self.original)).relocations]
        self.assertEqual(alias_targets, original_targets)
        with self.assertRaisesRegex(ValueError, 'First mismatch.*MZ_HEADER'):
            mismatch(self.original[:512], aliased, self.owner)

    def test_unsigned_fields_counts_lengths_and_target_bounds(self):
        cases = [
            ('e_cs', -1, 'unsigned 16-bit'), ('e_ip', 65536, 'unsigned 16-bit'),
            ('e_ss', True, 'unsigned 16-bit'), ('e_crlc', 105, 'relocation count'),
            ('e_lfarlc', 33, 'before relocation'), ('e_cparhdr', 31, 'header/padding'),
            ('e_cblp', 512, 'header/load-image'), ('e_cp', 0, 'header/load-image'),
        ]
        for field, value, reason in cases:
            with self.subTest(field=field):
                changed = copy.deepcopy(self.document)
                changed['fields'][field] = value
                with self.assertRaisesRegex(ValueError, reason):
                    encode_header(changed, len(self.original))
        changed = copy.deepcopy(self.document)
        changed['relocations'][0]['segment'] = 0xFFFF
        with self.assertRaisesRegex(ValueError, 'target outside load image'):
            encode_header(changed, len(self.original))
        with self.assertRaisesRegex(ValueError, 'Truncated MZ header'):
            MZ.parse_header(self.original[:511], len(self.original))

    def test_unknown_missing_and_malformed_source_data_rejected(self):
        changed = copy.deepcopy(self.document)
        changed['fields'].pop('e_ovno')
        with self.assertRaisesRegex(ValueError, 'exactly the 14'):
            encode_header(changed, len(self.original))
        changed = copy.deepcopy(self.document)
        changed['unexpected'] = 0
        with self.assertRaisesRegex(ValueError, 'format or keys'):
            encode_header(changed, len(self.original))
        changed = copy.deepcopy(self.document)
        changed['after_relocations_hex'] = 'GG'
        with self.assertRaisesRegex(ValueError, 'invalid hexadecimal'):
            encode_header(changed, len(self.original))
        changed = copy.deepcopy(self.document)
        changed['relocations'][0]['linear_target'] = 1
        with self.assertRaisesRegex(ValueError, 'expected segment and offset'):
            encode_header(changed, len(self.original))

    def test_full_final_page_trailing_file_bytes_and_duplicate_relocations(self):
        # A small independent fixture covers an exact 512-byte final page,
        # trailing non-load bytes, unsorted and duplicate relocation entries.
        fields = dict(zip(HEADER_FIELDS, (0x5A4D, 0, 2, 3, 4, 0, 0xFFFF, 0, 0, 0x1234, 0, 0, 30, 0)))
        doc = {'format': 'empires-mz-header-v1', 'fields': fields,
               'relocations': [{'offset': 40, 'segment': 0}, {'offset': 2, 'segment': 0}, {'offset': 40, 'segment': 0}],
               'before_relocations_hex': 'AB CD', 'after_relocations_hex': '12 34 ' * 11}
        encoded = encode_header(doc, 1100)
        self.assertEqual(encoded[30:42], struct.pack('<6H', 40, 0, 2, 0, 40, 0))
        self.assertEqual(struct.unpack_from('<H', encoded, 18)[0], 0x1234)
        image = encoded + bytes(1024 - len(encoded)) + b'X' * 76
        mz = MZ.parse(image)
        self.assertEqual(mz.declared_size, 1024)
        self.assertEqual(len(mz.load_image(image)), 960)
        self.assertEqual(len(mz.relocations), 3)
        self.assertEqual(encode_header(header_document(image), len(image)), encoded)

    def test_header_owner_must_start_at_zero(self):
        changed = copy.deepcopy(self.manifest)
        changed['regions'][1]['kind'] = 'MZ_HEADER'
        with self.assertRaisesRegex(ValueError, 'must begin at file offset zero'):
            validate_layout(changed)

    def test_changed_fields_reserved_bytes_and_padding_fail_before_compile(self):
        mutations = []
        for field in ('e_cs', 'e_csum'):
            changed = copy.deepcopy(self.document)
            changed['fields'][field] += 1
            mutations.append(changed)
        for key in ('before_relocations_hex', 'after_relocations_hex'):
            changed = copy.deepcopy(self.document)
            data = bytearray.fromhex(changed[key])
            data[0] ^= 1
            changed[key] = data.hex(' ')
            mutations.append(changed)
        with tempfile.TemporaryDirectory(dir=ROOT / 'build', prefix='test-mz-') as tmp:
            root = Path(tmp)
            for directory in ('layout', 'assets', 'build'):
                (root / directory).mkdir()
            (root / 'assets/AEPROG.EXE').write_bytes(self.original)
            manifest_path = root / 'layout/manifest.json'
            manifest_path.write_text(json.dumps(self.manifest))
            for changed in mutations:
                (root / 'layout/mz-header.json').write_text(json.dumps(changed))
                with patch('reconstruct.compile_sources') as compiler:
                    with self.assertRaisesRegex(ValueError, 'First mismatch.*MZ_HEADER'):
                        reconstruct(root, manifest_path, root / 'build', root / 'toolchain', Path('unused'))
                    compiler.assert_not_called()
                self.assertFalse((root / 'build/AEPROG.EXE').exists())
                self.assertFalse((root / 'build/report.json').exists())


if __name__ == '__main__':
    unittest.main()
