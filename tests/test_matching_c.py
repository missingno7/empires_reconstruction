"""Fresh source proof and negative controls for the first local C wave."""
import copy
from pathlib import Path
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from mz import MZ
from dos_runner import resolve_runner
from reconstruct import (read_json, compile_sources, read_object, bind_region, mismatch,
                         owned_library_modules)


class MatchingCTests(unittest.TestCase):
    def test_fresh_wave_and_codegen_negative_controls(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave1.json')
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        lock = read_json(ROOT / 'layout/toolchain.json')
        modules = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        owners = copy.deepcopy(recipe['owners'])
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            work = Path(temporary)
            wrong_order = copy.deepcopy(next(o for o in owners if o['id'] == 'F_68CF'))
            source = (ROOT / wrong_order['source']).read_bytes()
            self.assertEqual(source.count(b'f2 = f1 ='), 2)
            path = work / 'wrong.C'
            path.write_bytes(source.replace(b'f2 = f1 =', b'f1 = f2 ='))
            wrong_order.update(id='WRONG_ORDER', source=path.relative_to(ROOT).as_posix())
            wrong_frame = copy.deepcopy(next(o for o in owners if o['id'] == 'F_CB48'))
            wrong_frame['id'] = 'WRONG_FRAME'
            wrong_frame['build']['flags_append'] = ''
            receipts, _ = compile_sources(ROOT, owners + [wrong_order, wrong_frame], work / 'compiler',
                                          ROOT / 'toolchain', resolve_runner(lock), lock)
            checked = 0
            for owner in owners:
                module = read_object((work / 'compiler' / receipts[owner['id']]['object']).read_bytes())
                data, proof = bind_region(owner, module, MZ.parse(original), manifest['frames'], manifest['regions'], modules)
                mismatch(original[owner['start']:owner['end']], data, owner)
                checked += len(data)
                if owner['id'] == 'F_D60C':
                    self.assertEqual(data[-1], 0xc3)
                    raw = next(r for r in manifest['regions'] if r['start'] <= owner['end'] < r['end'])
                    self.assertEqual(raw['kind'], 'EXACT_DATA')
                    self.assertEqual(raw['build']['encoder'], 'zero-pad-v1')
                    self.assertEqual(original[owner['end']], 0)
            self.assertEqual(checked, 291)
            module = read_object((work / 'compiler' / receipts['WRONG_ORDER']['object']).read_bytes())
            data, _ = bind_region(wrong_order, module, MZ.parse(original), manifest['frames'], manifest['regions'], modules)
            with self.assertRaisesRegex(ValueError, 'First mismatch'):
                mismatch(original[wrong_order['start']:wrong_order['end']], data, wrong_order)
            module = read_object((work / 'compiler' / receipts['WRONG_FRAME']['object']).read_bytes())
            with self.assertRaisesRegex(ValueError, 'wrong compiled extent length'):
                bind_region(wrong_frame, module, MZ.parse(original), manifest['frames'], manifest['regions'], modules)
