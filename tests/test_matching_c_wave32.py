"""Fresh F_9B68 comparison and structured table round-trip controls."""
import copy
from pathlib import Path
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from exe_data import encode_data
from mz import MZ
from reconstruct import read_json, compile_sources, read_object, bind_region, mismatch, owned_library_modules


class MatchingCWave32Tests(unittest.TestCase):
    def test_fresh_routine_and_table_sources(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave32.json')
        code = next(o for o in recipe['owners'] if o['kind'] == 'MATCHING_C')
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        lock = read_json(ROOT / 'layout/toolchain.json')
        modules = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            work = Path(temporary)
            receipts, _ = compile_sources(ROOT, [code], work / 'compiler', ROOT / 'toolchain',
                                          Path(lock['dosbox_default']), lock)
            module = read_object((work / 'compiler' / receipts[code['id']]['object']).read_bytes())
            data, _ = bind_region(code, module, MZ.parse(original), manifest['frames'], manifest['regions'], modules)
            mismatch(original[code['start']:code['end']], data, code)
        for owner in recipe['owners']:
            if owner['kind'] != 'EXACT_DATA':
                continue
            source = read_json(ROOT / owner['source'])
            encoded = encode_data(source, owner['build']['encoder'])
            self.assertEqual(encoded, original[owner['start']:owner['end']])
            changed = copy.deepcopy(source)
            if source['format'] == 'fixed-records-v1':
                changed['records'][0] = '00' * source['record_size']
            else:
                changed['values'][0] ^= 1
            self.assertNotEqual(encode_data(changed, owner['build']['encoder']), encoded)
