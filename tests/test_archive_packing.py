"""Prove archive layout is calculated, and isolate construction from original fixtures."""
import copy
from pathlib import Path
import shutil
import struct
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from archive_recipe import recipe_from_layout, update_existing_recipe
from pack_archives import build_archive, pack, pack_blocks, validate_recipe, verify
from reconstruct import read_json, write_json
from reconstruct_archives import rebuild


class PackingRulesTests(unittest.TestCase):
    def test_offsets_follow_emitted_sizes_count_empty_slots_and_trailer(self):
        blocks = [b'\x47\x00abc', b'', b'\x20\x00z']
        data, offsets = pack_blocks(blocks, b'tail')
        self.assertEqual(offsets, [16, 21, 21, 24])
        self.assertEqual(data, struct.pack('<4I', *offsets) + b''.join(blocks) + b'tail')
        longer = [blocks[0] + b'xyz', *blocks[1:]]
        _, moved = pack_blocks(longer)
        self.assertEqual(moved, [16, 24, 24, 27])
        _, reordered = pack_blocks([blocks[2], blocks[0], blocks[1]])
        self.assertEqual(reordered, [16, 19, 24, 24])
        _, added = pack_blocks([*blocks, b'\x01\x00'])
        self.assertEqual(added, [20, 25, 25, 28, 30])
        self.assertEqual(pack_blocks([]), (b'\x04\x00\x00\x00', [4]))

    def test_recipe_rejects_historical_placement_and_verification_inputs(self):
        recipe = read_json(ROOT / 'recipes/archives/AE000.json')
        for key in ('offsets', 'original', 'expected_sha256', 'size'):
            changed = copy.deepcopy(recipe)
            changed[key] = 123
            with self.assertRaisesRegex(ValueError, 'recipe fields'):
                validate_recipe(changed)
        for key in ('start', 'end', 'offset', 'expected_sha256', 'length'):
            changed = copy.deepcopy(recipe)
            changed['resources'][0][key] = 123
            with self.assertRaisesRegex(ValueError, 'recipe fields'):
                validate_recipe(changed)
        changed = copy.deepcopy(recipe)
        changed['resources'][1]['id'] = changed['resources'][0]['id']
        with self.assertRaisesRegex(ValueError, 'unique'):
            validate_recipe(changed)

    def test_recipes_match_promoted_component_sources_without_layout_fields(self):
        for name in ('AE000', 'AE001'):
            recipe = read_json(ROOT / f'recipes/archives/{name}.json')
            self.assertEqual(recipe, recipe_from_layout(read_json(ROOT / f'layout/archives/{name}.json')))
            validate_recipe(recipe)


class IndependentPackingTests(unittest.TestCase):
    def setUp(self):
        (ROOT / 'build').mkdir(exist_ok=True)
        self.temp = tempfile.TemporaryDirectory(prefix='test-packing-', dir=ROOT / 'build')
        self.root = Path(self.temp.name)
        shutil.copytree(ROOT / 'recipes', self.root / 'recipes')
        (self.root / 'tools').mkdir()
        for name in ('pack_archives.py', 'resource_codecs.py', 'resource_formats.py'):
            shutil.copyfile(ROOT / 'tools' / name, self.root / 'tools' / name)
        for name in ('AE000', 'AE001'):
            recipe = read_json(self.root / f'recipes/archives/{name}.json')
            for entry in recipe['resources']:
                if 'source' in entry:
                    target = self.root / entry['source']
                    target.parent.mkdir(parents=True, exist_ok=True)
                    shutil.copyfile(ROOT / entry['source'], target)

    def tearDown(self):
        self.temp.cleanup()

    def add_verification_fixtures(self):
        shutil.copytree(ROOT / 'layout/archives', self.root / 'layout/archives')
        (self.root / 'assets').mkdir()
        for name in ('AE000', 'AE001'):
            shutil.copyfile(ROOT / f'assets/{name}.DAT', self.root / f'assets/{name}.DAT')

    def test_real_packing_needs_no_originals_or_fixed_manifest_and_matches_both(self):
        self.assertFalse((self.root / 'assets').exists())
        self.assertFalse((self.root / 'layout').exists())
        # Reject reads outside the component recipe/source/tool trees as well.
        read_bytes, read_text = Path.read_bytes, Path.read_text
        def allowed(path):
            path = path.resolve()
            self.assertTrue(path.is_relative_to(self.root.resolve()), path)
            self.assertIn(path.relative_to(self.root).parts[0], ('raw', 'recipes', 'tools'))
        def bytes_guard(path, *args, **kwargs):
            allowed(path)
            return read_bytes(path, *args, **kwargs)
        def text_guard(path, *args, **kwargs):
            allowed(path)
            return read_text(path, *args, **kwargs)
        with patch.object(Path, 'read_bytes', bytes_guard), patch.object(Path, 'read_text', text_guard):
            report = pack(self.root)
        self.assertEqual(sum(r['opaque_fallback_resources'] for r in report['archives'].values()), 193)
        for name in ('AE000', 'AE001'):
            self.assertEqual((self.root / f'build/packed/{name}.DAT').read_bytes(), (ROOT / f'assets/{name}.DAT').read_bytes())
            self.assertFalse(report['archives'][name]['historical_offsets_read'])
        self.add_verification_fixtures()
        rebuild(ROOT, self.root / 'fixed')
        with self.assertRaisesRegex(ValueError, 'must be separate'):
            verify(self.root, fixed_output=self.root / 'build/packed')
        result = verify(self.root, fixed_output=self.root / 'fixed')
        self.assertTrue(all(r['fixed_equal'] for r in result['archives'].values()))

    def test_changed_component_length_moves_following_offsets_without_fixture(self):
        recipe = read_json(self.root / 'recipes/archives/AE000.json')
        before, proof = build_archive(self.root, recipe)
        entry = recipe['resources'][0]
        self.assertEqual(entry['representation'], 'opaque-encoded-fallback')
        source = self.root / entry['source']
        source.write_bytes(source.read_bytes() + b'\x12\x34\x56')
        after, changed = build_archive(self.root, recipe)
        self.assertEqual(len(after), len(before) + 3)
        self.assertEqual(changed['generated_offsets'][0], proof['generated_offsets'][0])
        self.assertEqual(changed['generated_offsets'][1:], [x + 3 for x in proof['generated_offsets'][1:]])

    def test_missing_component_does_not_fall_back_and_invalidates_success(self):
        output = self.root / 'build/packed'
        output.mkdir(parents=True)
        for name in ('AE000.DAT', 'AE001.DAT', 'packing-report.json', 'verification.json'):
            (output / name).write_bytes(b'previous')
        (self.root / 'build/game-report.json').write_bytes(b'previous')
        recipe = read_json(self.root / 'recipes/archives/AE001.json')
        (self.root / recipe['resources'][0]['source']).unlink()
        with self.assertRaises(FileNotFoundError):
            pack(self.root)
        self.assertEqual(list(output.iterdir()), [])
        self.assertFalse((self.root / 'build/game-report.json').exists())

    def test_verification_rejects_changed_emission_and_removes_stale_verdict(self):
        self.add_verification_fixtures()
        recipe = read_json(self.root / 'recipes/archives/AE000.json')
        source = self.root / recipe['resources'][0]['source']
        source.write_bytes(source.read_bytes() + b'new size')
        pack(self.root)
        verdict = self.root / 'build/packed/verification.json'
        verdict.write_bytes(b'previous equality')
        with self.assertRaisesRegex(ValueError, 'first mismatch'):
            verify(self.root)
        self.assertFalse(verdict.exists())

    def test_promotion_updates_recipe_and_invalidates_packed_outputs(self):
        manifest = read_json(ROOT / 'layout/archives/AE000.json')
        manifest['resources'][0]['source'] = 'raw/AE000/new-candidate.bin'
        output = self.root / 'build/packed'
        output.mkdir(parents=True)
        for filename in ('packing-report.json', 'verification.json', 'AE000.DAT', 'AE001.DAT'):
            (output / filename).write_bytes(b'previous')
        update_existing_recipe(self.root, 'AE000', manifest)
        self.assertEqual(read_json(self.root / 'recipes/archives/AE000.json'), recipe_from_layout(manifest))
        self.assertEqual(list(output.iterdir()), [])


if __name__ == '__main__':
    unittest.main()
