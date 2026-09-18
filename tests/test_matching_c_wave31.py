"""Fresh critical-error handler pair and byte-storage evidence checks."""
import copy
from pathlib import Path
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from mz import MZ
from reconstruct import read_json, compile_sources, read_object, bind_region, mismatch, owned_library_modules
from storage_evidence import verify


class MatchingCWave31Tests(unittest.TestCase):
    def test_fresh_handler_pair(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        owners = read_json(ROOT / 'recipes/c/matching-wave31.json')['owners']
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        lock = read_json(ROOT / 'layout/toolchain.json')
        modules = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            work = Path(temporary)
            receipts, _ = compile_sources(ROOT, owners, work / 'compiler', ROOT / 'toolchain',
                                          Path(lock['dosbox_default']), lock)
            for owner in owners:
                module = read_object((work / 'compiler' / receipts[owner['id']]['object']).read_bytes())
                data, _ = bind_region(owner, module, MZ.parse(original), manifest['frames'], manifest['regions'], modules)
                mismatch(original[owner['start']:owner['end']], data, owner)

    def test_byte_storage_evidence_controls(self):
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        manifest = read_json(ROOT / 'layout/manifest.json')
        document = read_json(ROOT / 'docs/storage-binding-evidence.json')
        self.assertEqual(verify(document, original, manifest['frames'])['DS_C0C8'], 0xC0C8)
        changed = copy.deepcopy(document)
        item = next(o for o in changed['objects'] if o['id'] == 'DS_C0C8')
        item['observations'][1]['load_offset'] += 1
        with self.assertRaises(ValueError):
            verify(changed, original, manifest['frames'])
