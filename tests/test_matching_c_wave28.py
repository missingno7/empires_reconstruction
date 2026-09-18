"""Fresh F_21DB comparison and DS:96EE buffer evidence checks."""
import copy
from pathlib import Path
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from mz import MZ
from reconstruct import (read_json, compile_sources, read_object, bind_region,
                         mismatch, owned_library_modules)
from storage_evidence import verify


class MatchingCWave28Tests(unittest.TestCase):
    def test_fresh_match_and_loop_bound_mutant(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        owner = read_json(ROOT / 'recipes/c/matching-wave28.json')['owners'][0]
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        lock = read_json(ROOT / 'layout/toolchain.json')
        modules = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            work = Path(temporary)
            source = (ROOT / owner['source']).read_bytes()
            mutant_path = work / 'F_21DB_MUTANT.C'
            mutant_path.write_bytes(source.replace(b'i < 23', b'i < 22'))
            mutant = copy.deepcopy(owner)
            mutant.update(id='F_21DB_MUTANT', source=mutant_path.relative_to(ROOT).as_posix())
            receipts, _ = compile_sources(ROOT, [owner, mutant], work / 'compiler', ROOT / 'toolchain',
                                          Path(lock['dosbox_default']), lock)
            module = read_object((work / 'compiler' / receipts[owner['id']]['object']).read_bytes())
            data, _ = bind_region(owner, module, MZ.parse(original), manifest['frames'], manifest['regions'], modules)
            mismatch(original[owner['start']:owner['end']], data, owner)
            mutant_module = read_object((work / 'compiler' / receipts[mutant['id']]['object']).read_bytes())
            with self.assertRaises(ValueError):
                mutant_data, _ = bind_region(mutant, mutant_module, MZ.parse(original), manifest['frames'], manifest['regions'], modules)
                mismatch(original[mutant['start']:mutant['end']], mutant_data, mutant)

    def test_buffer_storage_evidence_is_independent(self):
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        manifest = read_json(ROOT / 'layout/manifest.json')
        document = read_json(ROOT / 'docs/storage-binding-evidence.json')
        self.assertEqual(verify(document, original, manifest['frames'])['DS_96EE'], 0x96EE)
        item = next(o for o in document['objects'] if o['id'] == 'DS_96EE')
        for variant in ('address', 'site', 'access', 'extent', 'missing'):
            changed = copy.deepcopy(document)
            changed_item = next(o for o in changed['objects'] if o['id'] == 'DS_96EE')
            first = changed_item['observations'][0]
            if variant == 'address': changed_item['offset'] += 1
            elif variant == 'site': first['load_offset'] += 1
            elif variant == 'access': first['access'] = 'read'
            elif variant == 'extent': first['function_extent']['end'] -= 1
            else: changed_item['observations'].pop()
            with self.subTest(variant=variant), self.assertRaises(ValueError):
                verify(changed, original, manifest['frames'])
