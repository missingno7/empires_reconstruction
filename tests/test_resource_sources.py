"""Structured bank/font identity and editable PNG-to-DOS component generation."""
from collections import Counter
import copy
from pathlib import Path
import struct
import sys
import tempfile
import unittest
import zlib

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from bitmap_sources import encode_source, export_source, image_path
from indexed_png import SIGNATURE, PALETTE, chunk, paeth, read_png, write_png
from pack_archives import build_archive
from reconstruct import read_json
from resource_codecs import decode_payload
from resource_formats import (bank_document, bitmap_document, decode_document, encode_bank,
                              encode_document, encode_font, encode_sequence, font_document, sequence_document)


class NestedResourceTests(unittest.TestCase):
    def test_all_real_structured_payloads_and_nested_records(self):
        formats, records = Counter(), Counter()
        for name in ('AE000', 'AE001'):
            manifest = read_json(ROOT / f'layout/archives/{name}.json')
            data = (ROOT / f'assets/{name}.DAT').read_bytes()
            for entry in manifest['resources']:
                payload = decode_payload(data[entry['start'] + 2:entry['end']], entry['flags'])
                document = decode_document(name, entry, payload)
                if document is None:
                    continue
                self.assertEqual(encode_document(document), payload, entry['id'])
                formats[document['format']] += 1
                records.update(r['format'] for r in document.get('records', []))
        self.assertEqual(dict(formats), {'bitmap4-json-v1': 49, 'level-parts-json-v1': 20,
                                        'resource-bank16-json-v1': 75, 'bitmap-sequence-json-v1': 7, 'font1-json-v1': 2})
        self.assertEqual(dict(records), {'bitmap4-record-v1': 596, 'opaque-record-v1': 100, 'bitmap1-record-v1': 182})

    def test_bank_empty_slots_unknown_records_trailer_and_derived_offsets(self):
        data = struct.pack('<4H', 8, 11, 11, 15) + b'XYZabcdTAIL'
        source = bank_document(data)
        self.assertEqual(encode_bank(source), data)
        self.assertEqual(source['records'][1]['format'], 'empty-record-v1')
        source['records'][0]['bytes'] += '12'
        changed = encode_bank(source)
        self.assertEqual(struct.unpack_from('<4H', changed), (8, 12, 12, 16))
        self.assertTrue(changed.endswith(b'abcdTAIL'))
        for bad in (b'\x03\x00x', struct.pack('<3H', 6, 8, 7) + b'ab', struct.pack('<2H', 4, 7) + b'a'):
            with self.assertRaises(ValueError):
                bank_document(bad)
        source['records'][0]['bytes'] = 'ab' * 65536
        with self.assertRaisesRegex(ValueError, '16 bits'):
            encode_bank(source)

    def test_font_offsets_are_emitted_sizes_and_padding_bits_survive(self):
        font = {'format': 'font1-json-v1', 'control': 10, 'line_height': 2,
                'glyphs': [{'width': 1, 'rows': ['ff', '7f']}, {'width': 9, 'rows': ['aaff', '55ff']}],
                'preserved_trailing': '1234'}
        original = encode_font(font)
        self.assertEqual(font_document(original), font)
        self.assertEqual(original[5:9], b'\x00\x02\x00\x00')
        changed = copy.deepcopy(font)
        changed['glyphs'][0] = {'width': 9, 'rows': ['ffff', '7fff']}
        self.assertEqual(encode_font(changed)[5:9], b'\x00\x04\x00\x00')
        corrupt = bytearray(original)
        corrupt[6] = 3
        with self.assertRaisesRegex(ValueError, 'contiguous'):
            font_document(corrupt)
        with self.assertRaisesRegex(ValueError, 'tables'):
            font_document(original[:5])

    def test_sequences_require_complete_ownership(self):
        record = b'G\x05' + bytes(range(32)) + b'\x01\x01\xab'
        self.assertEqual(encode_sequence(sequence_document(record * 2)), record * 2)
        for data in (record + b'X', record[:-1], b'x' + record[1:]):
            with self.assertRaises(ValueError):
                sequence_document(data)


class PNGSourceTests(unittest.TestCase):
    def test_four_bit_png_preserves_exact_indices(self):
        rows = [bytes.fromhex('0123456789abcdef'), bytes.fromhex('fedcba9876543210')]
        self.assertEqual(read_png(write_png(16, rows)), (16, rows))

    def test_editor_eight_bit_png_all_five_filters(self):
        raw = [bytes((x + y) % 16 for x in range(8)) for y in range(5)]
        filtered, previous = bytearray(), bytes(8)
        for method, row in enumerate(raw):
            filtered.append(method)
            for x, value in enumerate(row):
                left, up, upper_left = row[x - 1] if x else 0, previous[x], previous[x - 1] if x else 0
                predictor = (0, left, up, (left + up) // 2, paeth(left, up, upper_left))[method]
                filtered.append((value - predictor) & 255)
            previous = row
        png = (SIGNATURE + chunk(b'IHDR', struct.pack('>IIBBBBB', 8, 5, 8, 3, 0, 0, 0))
               + chunk(b'PLTE', PALETTE) + chunk(b'IDAT', zlib.compress(filtered)) + chunk(b'IEND', b''))
        width, rows = read_png(png)
        self.assertEqual(width, 8)
        self.assertEqual(rows, [bytes((r[x] << 4) | r[x + 1] for x in range(0, 8, 2)) for r in raw])

    def test_png_corruption_palette_remap_rgb_and_extra_stream_rejected(self):
        original = write_png(2, [b'\xab'])
        corrupt = bytearray(original)
        corrupt[20] ^= 1
        for bad in (original[:-1], original + b'x', bytes(corrupt)):
            with self.assertRaises(ValueError):
                read_png(bad)
        def fixture(depth=4, colour=3, palette=PALETTE, data=b'\0\xab', extra=b''):
            return (SIGNATURE + chunk(b'IHDR', struct.pack('>IIBBBBB', 2, 1, depth, colour, 0, 0, 0))
                    + chunk(b'PLTE', palette) + chunk(b'IDAT', zlib.compress(data) + extra) + chunk(b'IEND', b''))
        for bad in (fixture(palette=PALETTE[::-1]), fixture(colour=2, depth=8),
                    fixture(extra=zlib.compress(b'X')), fixture(data=b'\5\xab'), fixture(depth=8, data=b'\0\x10\x01')):
            with self.assertRaises(ValueError):
                read_png(bad)

    def test_png_edit_builds_changed_dos_resource_and_moves_later_offsets(self):
        with tempfile.TemporaryDirectory(prefix='test-png-source-', dir=ROOT / 'build') as directory:
            root = Path(directory)
            payload = bytes(range(32)) + b'\x02\x02\x01\x23\x45\x67'
            metadata_path = root / 'bitmap.json'
            metadata = export_source(bitmap_document(payload), metadata_path)
            self.assertEqual(encode_source(metadata, metadata_path), payload)
            recipe = {'format': 'empires-dat-build-v1', 'packing': 'ordered-contiguous-le32-v1', 'trailing': None,
                      'resources': [{'id': 'image', 'rtype': 71, 'flags': 3, 'source': 'bitmap.json',
                                     'representation': 'bitmap4-png-v1', 'encoder': 'greedy-rle-pair-span-v1'},
                                    {'id': 'empty', 'representation': 'empty'}]}
            original, before = build_archive(root, recipe)
            # Change dimensions and pixel indices using the same editable PNG input.
            image_path(metadata, metadata_path).write_bytes(write_png(8, [bytes.fromhex('01234567')] * 5))
            edited, after = build_archive(root, recipe)
            self.assertNotEqual(original, edited)
            start, end = after['generated_offsets'][:2]
            rebuilt_payload = decode_payload(edited[start + 2:end], 3)
            self.assertEqual(rebuilt_payload, bytes(range(32)) + b'\x04\x05' + bytes.fromhex('01234567') * 5)
            self.assertEqual(after['generated_offsets'][1] - before['generated_offsets'][1], len(edited) - len(original))
            self.assertEqual(after['generated_offsets'][1], after['generated_offsets'][2])

    def test_png_source_path_cannot_escape_metadata_directory(self):
        metadata = {'format': 'bitmap4-png-v1', 'ega_cga_table': [0] * 16, 'vga_table': [0] * 16, 'image': '../outside.png'}
        with self.assertRaisesRegex(ValueError, 'filename beside'):
            image_path(metadata, ROOT / 'raw/test.json')


if __name__ == '__main__':
    unittest.main()
