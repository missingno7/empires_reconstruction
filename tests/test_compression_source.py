"""Exact compression syntax and independent archive source reconstruction."""
from pathlib import Path
import struct
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from compression_source import FORMAT, decode_source, encode_source
from resource_codecs import BitWriter, unpack_pair_span
from probe_compression_sources import export, build


class CompressionSourceTests(unittest.TestCase):
    def test_explicit_width_duplicate_pairs_and_literal_edit(self):
        writer = BitWriter()
        # Two identical dictionary entries; preserve the second reference and
        # an unnecessary but valid early width escape exactly.
        writer.write(256, 9)
        for code in (65, 66, 65, 66, 258):
            writer.write(code, 10)
        encoded = struct.pack('<H', 6) + writer.finish()
        data, source = decode_source(encoded)
        self.assertEqual(data, b'ABABAB')
        self.assertEqual(source['instructions'], [['widen', 1], ['literal', 4], ['pair', 1]])
        self.assertEqual(encode_source(data, source), encoded)
        changed = encode_source(b'CDCDCD', source)
        self.assertEqual(unpack_pair_span(changed), b'CDCDCD')
        with self.assertRaisesRegex(ValueError, 'differs from decoded'):
            encode_source(b'ABABAC', source)

    def test_invalid_instructions_and_missing_coverage(self):
        for instructions in ([['pair', 0]], [['literal', True]], [['literal', 0]],
                             [['literal', 3]], [['unknown', 1]], [['widen', 2]],
                             [['literal', 1]], [['literal', 2], ['widen', 1]],
                             [['widen', 1]] * 8 + [['literal', 2]]):
            with self.subTest(instructions=instructions), self.assertRaises(ValueError):
                encode_source(b'AB', {'format': FORMAT, 'instructions': instructions})
        self.assertEqual(encode_source(b'', {'format': FORMAT, 'instructions': []}), b'\0\0')
        with self.assertRaises(ValueError):
            decode_source(b'\0\0\0')
        with self.assertRaises(ValueError):
            encode_source(bytes(65536), {'format': FORMAT, 'instructions': []})

    def test_all_archives_build_without_external_reads(self):
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            directory = Path(temporary) / 'sources'
            report = export(ROOT, directory)
            self.assertEqual(report['explicit_compression_plans'], 182)
            self.assertEqual(report['structured_payloads'], 153)
            self.assertEqual(report['opaque_decoded_payloads'], 67)
            self.assertFalse(report['historical_compressor_policy_recovered'])
            expected = {name: (ROOT / f'assets/{name}.DAT').read_bytes() for name in ('AE000', 'AE001')}
            original_open = Path.open
            def guarded_open(path, *args, **kwargs):
                self.assertTrue(path.resolve().is_relative_to(directory.resolve()), str(path))
                return original_open(path, *args, **kwargs)
            with patch.object(Path, 'open', guarded_open):
                self.assertEqual(build(directory), expected)
            with self.assertRaisesRegex(ValueError, 'already exists'):
                export(ROOT, directory)
