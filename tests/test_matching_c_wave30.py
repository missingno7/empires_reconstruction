"""Fresh F_A658 comparison and a codegen negative control."""
import copy
from pathlib import Path
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from mz import MZ
from dos_runner import resolve_runner
from reconstruct import (read_json, compile_sources, read_object, bind_region,
                         mismatch, owned_library_modules)


class MatchingCWave30Tests(unittest.TestCase):
    def test_fresh_code_and_compiled_data(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        code = next(o for o in manifest['regions'] if o['id'] == 'F_A658')
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        lock = read_json(ROOT / 'layout/toolchain.json')
        modules = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            work = Path(temporary)
            receipts, _ = compile_sources(ROOT, [code], work / 'compiler', ROOT / 'toolchain',
                                          resolve_runner(lock), lock)
            module = read_object((work / 'compiler' / receipts[code['id']]['object']).read_bytes())
            text, _ = bind_region(code, module, MZ.parse(original), manifest['frames'], manifest['regions'], modules)
            mismatch(original[code['start']:code['end']], text, code)
            # F_A658 was normalized to reference the shared, externally owned
            # `menu_empty_record` struct instead of a local string literal
            # argument; it no longer compiles its own _DATA segment or a
            # DATA_010FA5_RECORDS module-segment binding.
            self.assertEqual(module.segments.get('_DATA'), None)
            self.assertEqual(code['build']['module_segments'], {})

            mutant_path = work / 'F_A658_MUTANT.C'
            self.assertEqual((ROOT / code['source']).read_bytes().count(b'sel = 0x10;'), 1)
            mutant_path.write_bytes((ROOT / code['source']).read_bytes().replace(
                b'sel = 0x10;', b'sel = 0x11;'))
            mutant = copy.deepcopy(code)
            mutant.update(id='F_A658_MUTANT', source=mutant_path.relative_to(ROOT).as_posix())
            mutant_receipts, _ = compile_sources(ROOT, [mutant], work / 'mutant', ROOT / 'toolchain',
                                                  resolve_runner(lock), lock)
            mutant_module = read_object((work / 'mutant' / mutant_receipts[mutant['id']]['object']).read_bytes())
            with self.assertRaises(ValueError):
                mutant_text, _ = bind_region(mutant, mutant_module, MZ.parse(original), manifest['frames'],
                                             manifest['regions'], modules)
                mismatch(original[mutant['start']:mutant['end']], mutant_text, mutant)


if __name__ == '__main__':
    unittest.main()
