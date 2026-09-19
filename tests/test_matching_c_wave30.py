"""Fresh F_A658 text/data comparison and initializer negative control."""
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
                                          Path(lock['dosbox_default']), lock)
            module = read_object((work / 'compiler' / receipts[code['id']]['object']).read_bytes())
            text, _ = bind_region(code, module, MZ.parse(original), manifest['frames'], manifest['regions'], modules)
            mismatch(original[code['start']:code['end']], text, code)
            self.assertEqual(module.segment_bytes('_DATA'), b'\0')
            self.assertEqual(code['build']['module_segments']['_DATA']['owner'],
                             'DATA_010FA5_RECORDS')

            mutant_path = work / 'F_A658_MUTANT.C'
            mutant_path.write_bytes((ROOT / code['source']).read_bytes().replace(b'text("", 1)', b'text("x", 1)'))
            mutant = copy.deepcopy(code)
            mutant.update(id='F_A658_MUTANT', source=mutant_path.relative_to(ROOT).as_posix())
            mutant_receipts, _ = compile_sources(ROOT, [mutant], work / 'mutant', ROOT / 'toolchain',
                                                  Path(lock['dosbox_default']), lock)
            mutant_module = read_object((work / 'mutant' / mutant_receipts[mutant['id']]['object']).read_bytes())
            self.assertNotEqual(mutant_module.segment_bytes('_DATA'), module.segment_bytes('_DATA'))
