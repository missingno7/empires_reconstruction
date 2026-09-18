"""Archive identity, codec policy, structured payloads, and failed-build publication."""
import copy
import json
from pathlib import Path
import random
import shutil
import struct
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from dat_archive import assemble_archive, make_manifest, read_offsets, validate_manifest
from reconstruct import read_json, write_json
from reconstruct_archives import NAMES, prepare, rebuild
from resource_codecs import BitWriter, decode_payload, encode_payload, pack_pair_span, pack_rle, unpack_pair_span, unpack_rle
from resource_formats import bitmap_document, encode_bitmap, level_document, encode_level
from probe_archive_codecs import probe as probe_codecs
from reconstruct_game import exe_metrics


class CodecTests(unittest.TestCase):
    def test_rle_boundary_runs_literals_and_control_zero(self):
        self.assertEqual(unpack_rle(b'\x00Q\xffA\x80B\x03xyz'), b'QAA' + b'B' * 129 + b'xyz')
        self.assertEqual(pack_rle(b'A' * 130), b'\x80A\x01A')
        literals = bytes(range(128))
        self.assertEqual(pack_rle(literals), b'\x7f' + literals[:127] + b'\x01\x7f')
        for bad in (b'\x03ab', b'\x80', b'\x00'):
            with self.assertRaises(ValueError):
                unpack_rle(bad)
        with self.assertRaisesRegex(ValueError, 'limit'):
            unpack_rle(b'\x80x', max_output=128)

    def test_hand_authored_pair_span_and_width_escape(self):
        # 'a','b',dictionary[0] yields abab. Codes are explicitly specified,
        # independently of the compressor's dictionary search policy.
        writer = BitWriter()
        for code in (97, 98, 257):
            writer.write(code, 9)
        encoded = struct.pack('<H', 4) + writer.finish()
        self.assertEqual(unpack_pair_span(encoded), b'abab')
        self.assertEqual(pack_pair_span(b'abab'), encoded)
        writer = BitWriter()
        writer.write(256, 9)
        writer.write(65, 10)
        self.assertEqual(unpack_pair_span(b'\x01\x00' + writer.finish()), b'A')
        for bad in (encoded[:-1], encoded + b'\x00', encoded[:-1] + bytes([encoded[-1] | 1]), b'\x05\x00' + encoded[2:]):
            with self.assertRaises(ValueError):
                unpack_pair_span(bad)
        writer = BitWriter()
        writer.write(257, 9)
        with self.assertRaisesRegex(ValueError, 'reference'):
            unpack_pair_span(b'\x02\x00' + writer.finish())

    def test_candidate_codec_round_trips_width_growth(self):
        generator = random.Random(34071)
        data = bytes(generator.randrange(256) for _ in range(4000))
        for flags in range(4):
            self.assertEqual(decode_payload(encode_payload(data, flags), flags), data)
        with self.assertRaisesRegex(ValueError, '16-bit'):
            pack_pair_span(bytes(65536))
        with self.assertRaisesRegex(ValueError, 'flags'):
            decode_payload(b'', 4)

    def test_real_codec_policy_counts_and_exact_streams(self):
        counts = {'rle': 0, 'pair': 0, 'compressed_exact': 0, 'plain': 0}
        for name in NAMES:
            manifest = read_json(ROOT / f'layout/archives/{name}.json')
            original = (ROOT / f'assets/{name}.DAT').read_bytes()
            for entry in manifest['resources']:
                encoded = original[entry['start'] + 2:entry['end']]
                decoded = decode_payload(encoded, entry['flags'])
                if entry['flags'] & 2:
                    stage = unpack_pair_span(encoded)
                    self.assertEqual(int.from_bytes(encoded[:2], 'little'), len(stage))
                    counts['pair'] += 1
                else:
                    stage = encoded
                if entry['flags'] & 1:
                    self.assertEqual(pack_rle(decoded), stage, entry['id'])
                    counts['rle'] += 1
                if entry['flags']:
                    counts['compressed_exact'] += encode_payload(decoded, entry['flags']) == encoded
                else:
                    counts['plain'] += 1
        self.assertEqual(counts, {'rle': 155, 'pair': 182, 'compressed_exact': 26, 'plain': 38})


class ArchiveLayoutTests(unittest.TestCase):
    def test_duplicate_empty_slot_and_trailer_preserved(self):
        data = struct.pack('<4I', 16, 19, 19, 23) + b'\x47\x00X\x20\x00YZtail'
        manifest = make_manifest('TEST', data)
        self.assertEqual(manifest['resources'][1]['kind'], 'EMPTY_RESOURCE')
        payloads = {'TEST_000': b'X', 'TEST_001': b'', 'TEST_002': b'YZ'}
        self.assertEqual(assemble_archive(manifest, payloads, b'tail'), data)
        for changed in ({**payloads, 'TEST_001': b'X'}, {**payloads, 'TEST_000': b'Z'}, {'TEST_000': b'X'}):
            with self.assertRaises(ValueError):
                assemble_archive(manifest, changed, b'tail')
        with self.assertRaisesRegex(ValueError, 'trailing'):
            assemble_archive(manifest, payloads, b'taiL')

    def test_invalid_tables_ownership_and_headers(self):
        for bad in (b'', b'\x05\x00\x00\x00x', struct.pack('<3I', 12, 14, 13) + b'xx',
                    struct.pack('<2I', 8, 9) + b'x', struct.pack('<2I', 8, 12) + b'xx'):
            with self.assertRaises(ValueError):
                read_offsets(bad)
        data = struct.pack('<2I', 8, 11) + b'\x47\x00X'
        manifest = make_manifest('TEST', data)
        for field, value in (('start', 9), ('end', 10), ('rtype', 256), ('flags', -1), ('index', 1)):
            changed = copy.deepcopy(manifest)
            changed['resources'][0][field] = value
            with self.assertRaises(ValueError):
                validate_manifest(changed)
        changed = copy.deepcopy(manifest)
        changed['resources'][0]['rtype'] = 1
        with self.assertRaisesRegex(ValueError, 'resource bytes mismatch'):
            assemble_archive(changed, {'TEST_000': b'X'})

    def test_structured_bitmaps_levels_preserve_all_bytes(self):
        counts = {'bitmap': 0, 'level': 0}
        for name in NAMES:
            manifest = read_json(ROOT / f'layout/archives/{name}.json')
            original = (ROOT / f'assets/{name}.DAT').read_bytes()
            for entry in manifest['resources']:
                payload = decode_payload(original[entry['start'] + 2:entry['end']], entry['flags'])
                if entry['rtype'] == 0x47:
                    self.assertEqual(encode_bitmap(bitmap_document(payload)), payload)
                    counts['bitmap'] += 1
                elif name == 'AE001' and entry['index'] < 20:
                    self.assertEqual(encode_level(level_document(payload)), payload)
                    counts['level'] += 1
        self.assertEqual(counts, {'bitmap': 49, 'level': 20})

    def test_structured_format_invalid_lengths_and_preserved_unknowns(self):
        payload = bytes(range(32)) + b'\x01\x02\xab\xcd'
        document = bitmap_document(payload)
        document['pixel_rows'][0] = 'a'
        with self.assertRaises(ValueError):
            encode_bitmap(document)
        document = level_document(bytes(range(256)) * 102 + bytes(range(24)))
        document['parts'][0]['rooms'][0]['preserved_tail'] = '00'
        with self.assertRaisesRegex(ValueError, 'room tail'):
            encode_level(document)


class ArchiveBuildTests(unittest.TestCase):
    def setUp(self):
        (ROOT / 'build').mkdir(exist_ok=True)
        self.temp = tempfile.TemporaryDirectory(prefix='test-archives-', dir=ROOT / 'build')
        self.root = Path(self.temp.name)
        shutil.copytree(ROOT / 'layout/archives', self.root / 'layout/archives')
        (self.root / 'assets').mkdir()
        (self.root / 'tools').mkdir()
        for name in NAMES:
            shutil.copyfile(ROOT / f'assets/{name}.DAT', self.root / f'assets/{name}.DAT')
        for name in ('resource_codecs.py', 'resource_formats.py', 'bitmap_sources.py', 'indexed_png.py', 'dat_archive.py', 'reconstruct_archives.py'):
            shutil.copyfile(ROOT / f'tools/{name}', self.root / f'tools/{name}')
        prepare(self.root)

    def tearDown(self):
        self.temp.cleanup()

    def test_clean_archive_bootstrap_builds_without_upstream(self):
        reports = rebuild(self.root)
        self.assertEqual(sum(r['resources_exact_matching'] for r in reports.values()), 27)
        self.assertEqual(sum(r['structured_asset_sources'] for r in reports.values()), 27)
        for name in NAMES:
            self.assertEqual((self.root / f'build/{name}.DAT').read_bytes(), (self.root / f'assets/{name}.DAT').read_bytes())

    def test_raw_corruption_removes_previous_archive_and_game_success(self):
        output = self.root / 'build'
        output.mkdir()
        for name in ('AE000.DAT', 'AE001.DAT', 'archives-report.json', 'game-report.json'):
            (output / name).write_bytes(b'stale success')
        manifest = read_json(self.root / 'layout/archives/AE001.json')
        entry = next(r for r in manifest['resources'] if r['kind'] == 'RAW_RESOURCE')
        source = self.root / entry['source']
        data = bytearray(source.read_bytes())
        data[5] ^= 1
        source.write_bytes(data)
        with self.assertRaisesRegex(ValueError, entry['id']):
            rebuild(self.root)
        self.assertEqual(list(output.iterdir()), [])

    def test_structured_pixel_mutation_is_rejected(self):
        manifest = read_json(self.root / 'layout/archives/AE000.json')
        entry = next(r for r in manifest['resources'] if r.get('source_format') == 'bitmap4-png-v1')
        source = self.root / entry['source']
        document = read_json(source)
        document['vga_table'][0] ^= 1
        write_json(source, document)
        with self.assertRaisesRegex(ValueError, 'decoded source identity'):
            rebuild(self.root)

    def test_failed_promotion_preserves_manifests_and_existing_success(self):
        path = self.root / 'layout/archives/AE001.json'
        manifest = read_json(path)
        manifest['original']['sha256'] = '0' * 64
        write_json(path, manifest)
        before = {name: (self.root / f'layout/archives/{name}.json').read_bytes() for name in NAMES}
        output = self.root / 'build'
        output.mkdir()
        (output / 'archives-report.json').write_bytes(b'previous')
        with self.assertRaisesRegex(ValueError, 'original archive identity'):
            probe_codecs(self.root, promote=True)
        for name in NAMES:
            self.assertEqual((self.root / f'layout/archives/{name}.json').read_bytes(), before[name])
        self.assertEqual((output / 'archives-report.json').read_bytes(), b'previous')

    def test_exe_classification_does_not_double_count_overlapping_evidence(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        counts = {}
        for owner in manifest['regions']:
            counts[owner['kind']] = counts.get(owner['kind'], 0) + owner['end'] - owner['start']
        metrics = exe_metrics(ROOT, manifest, {'bytes_by_kind': counts, 'total_bytes': manifest['original']['size']})
        covered = set()
        for row in metrics['unresolved_machine_ranges']:
            covered.update(range(row['start'], row['end']))
        self.assertEqual(metrics['unresolved_machine_code_bytes'], len(covered))
        self.assertEqual(metrics['raw_unknown_bytes'] + len(covered), counts['RAW'])


if __name__ == '__main__':
    unittest.main()
