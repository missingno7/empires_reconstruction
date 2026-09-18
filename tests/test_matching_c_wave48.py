"""Fresh proof for F_9D8E and its relocated initialized C data segment."""
from pathlib import Path
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from exe_data import encode_data
from mz import MZ
from reconstruct import (read_json, compile_sources, read_object, bind_region,
                         compiled_data, mismatch, owned_library_modules)


class MatchingCWave48Tests(unittest.TestCase):
    def test_fresh_code_data_text_and_relocation(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave48.json')
        code = next(o for o in recipe['owners'] if o['id'] == 'F_9D8E')
        text = next(o for o in recipe['owners'] if o['id'] == 'TEXT_118C')
        data_owner = next(o for o in recipe['owners'] if o['id'] == 'DATA_125D')
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        lock = read_json(ROOT / 'layout/toolchain.json')
        modules = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            receipts, _ = compile_sources(ROOT, [code], Path(temporary), ROOT / 'toolchain',
                                          Path(lock['dosbox_default']), lock)
            module = read_object((Path(temporary) / receipts[code['id']]['object']).read_bytes())
            modules[code['id']] = module
            code_bytes, code_proof = bind_region(code, module, MZ.parse(original),
                                                 manifest['frames'], manifest['regions'], modules)
            mismatch(original[code['start']:code['end']], code_bytes, code)
            self.assertEqual(len(code_bytes), 62)
            self.assertEqual(len(code_proof['fixups']), 5)
            text_bytes = encode_data(read_json(ROOT / text['source']), text['build']['encoder'])
            mismatch(original[text['start']:text['end']], text_bytes, text)
            data_bytes, data_proof = compiled_data(data_owner, manifest['regions'], modules,
                                                   MZ.parse(original), manifest['frames'])
            mismatch(original[data_owner['start']:data_owner['end']], data_bytes, data_owner)
            self.assertEqual(len(data_bytes), 20)
            self.assertEqual(len(data_proof['fixups']), 1)
            self.assertEqual(data_proof['load_relocations'], [68758])


if __name__ == '__main__':
    unittest.main()
