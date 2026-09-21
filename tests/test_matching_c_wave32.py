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
from dos_runner import resolve_runner
from reconstruct import bind_region, compile_sources, mismatch, owned_library_modules, read_json, read_object


class MatchingCWave32Tests(unittest.TestCase):
    def test_fresh_routine_and_table_sources(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave32.json')
        code = next(o for o in recipe['owners'] if o['kind'] == 'MATCHING_C')
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        lock = read_json(ROOT / 'layout/toolchain.json')
        modules = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        # F_9B68 now belongs to the grouped multi-source module C_9A0E_9D79
        # (layout/production-plan.json) and no longer compiles standalone;
        # compile the whole module's concatenated sources the way the
        # production build does, pinned to the deterministic DOSBox runner
        # (an auto-selected MS-DOS Player backend can legitimately emit a
        # different byte or two here).
        plan = read_json(ROOT / 'layout/production-plan.json')['modules']
        group = next(m for m in plan if m['id'] == 'C_9A0E_9D79')
        self.assertIn(code['id'], group['members'])
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            work = Path(temporary)
            concat = work / f"{group['id']}.C"
            concat.write_bytes(b'\r\n'.join((ROOT / s).read_bytes() for s in group['sources']))
            compile_owner = {'id': group['id'], 'kind': 'MATCHING_C',
                             'source': concat.relative_to(ROOT).as_posix(),
                             'build': {'flags_append': group['build'].get('flags_append', '')}}
            receipts, _ = compile_sources(ROOT, [compile_owner], work / 'compiler', ROOT / 'toolchain',
                                          resolve_runner(lock), lock)
            module = read_object((work / 'compiler' / receipts[group['id']]['object']).read_bytes())
            owner = next(r for r in manifest['regions'] if r['id'] == code['id'])
            data, _ = bind_region(owner, module, MZ.parse(original), manifest['frames'], manifest['regions'], modules)
            mismatch(original[owner['start']:owner['end']], data, owner)
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
