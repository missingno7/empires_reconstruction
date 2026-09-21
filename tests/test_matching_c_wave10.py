"""Independent terminated text must remain editable and byte-exact."""
import copy
from pathlib import Path
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from exe_data import encode_data, decode_data, TEXT_FORMAT
from mz import MZ
from dos_runner import resolve_runner
from reconstruct import (read_json, compile_sources, read_object, bind_region,
                         mismatch, owned_library_modules)


class MatchingCWave10Tests(unittest.TestCase):
    def test_text_roundtrip_and_invalid_sources(self):
        for value in (b'\0', b'\x1c\x1b to Cycle\0', b'Start New Game\0'):
            self.assertEqual(encode_data(decode_data(value, TEXT_FORMAT), TEXT_FORMAT), value)
        for value in (b'unterminated', b'two\0strings\0', b'\xff\0'):
            with self.assertRaises(ValueError):
                decode_data(value, TEXT_FORMAT)
        for document in ({'format': TEXT_FORMAT, 'text': 'a\0b'},
                         {'format': TEXT_FORMAT, 'text': chr(128)},
                         {'format': TEXT_FORMAT, 'text': 12},
                         {'format': TEXT_FORMAT, 'text': 'x', 'padding': 0}):
            with self.assertRaises(ValueError):
                encode_data(document, TEXT_FORMAT)

    def test_fresh_code_text_and_mutated_components(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        owners = read_json(ROOT / 'recipes/c/matching-wave10.json')['owners']
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        code = next(o for o in owners if o['kind'] == 'MATCHING_C')
        lock = read_json(ROOT / 'layout/toolchain.json')
        modules = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            work = Path(temporary)
            receipts, _ = compile_sources(ROOT, [code], work, ROOT / 'toolchain',
                                          resolve_runner(lock), lock)
            module = read_object((work / receipts[code['id']]['object']).read_bytes())
            data, _ = bind_region(code, module, MZ.parse(original), manifest['frames'], manifest['regions'], modules)
            mismatch(original[code['start']:code['end']], data, code)
            self.assertEqual(len(data), 203)
            total = 0
            for owner in owners:
                if owner['kind'] != 'EXACT_DATA':
                    continue
                document = read_json(ROOT / owner['source'])
                data = encode_data(document, TEXT_FORMAT)
                mismatch(original[owner['start']:owner['end']], data, owner)
                total += len(data)
                changed = {**document, 'text': document['text'].swapcase()}
                with self.assertRaisesRegex(ValueError, 'First mismatch'):
                    mismatch(original[owner['start']:owner['end']], encode_data(changed, TEXT_FORMAT), owner)
            self.assertEqual(total, 27)
            altered = copy.deepcopy(manifest['regions'])
            text = next(o for o in altered if o['id'] == 'TEXT_1660')
            text['start'] += 1
            text['end'] += 1
            changed, _ = bind_region(code, module, MZ.parse(original), manifest['frames'], altered, modules)
            with self.assertRaisesRegex(ValueError, 'First mismatch'):
                mismatch(original[code['start']:code['end']], changed, code)
