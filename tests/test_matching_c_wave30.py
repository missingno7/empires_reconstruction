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
                         compiled_data, mismatch, owned_library_modules)


class MatchingCWave30Tests(unittest.TestCase):
    def test_fresh_code_and_compiled_data(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        owners = read_json(ROOT / 'recipes/c/matching-wave30.json')['owners']
        code = next(o for o in owners if o['kind'] == 'MATCHING_C')
        data_owner = next(o for o in owners if o['kind'] == 'EXACT_DATA')
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
            data, _ = compiled_data(data_owner, manifest['regions'], {code['id']: module}, MZ.parse(original))
            mismatch(original[data_owner['start']:data_owner['end']], data, data_owner)

            mutant_path = work / 'F_A658_MUTANT.C'
            mutant_path.write_bytes((ROOT / code['source']).read_bytes().replace(b'text("", 1)', b'text("x", 1)'))
            mutant = copy.deepcopy(code)
            mutant.update(id='F_A658_MUTANT', source=mutant_path.relative_to(ROOT).as_posix())
            mutant_receipts, _ = compile_sources(ROOT, [mutant], work / 'mutant', ROOT / 'toolchain',
                                                  Path(lock['dosbox_default']), lock)
            mutant_module = read_object((work / 'mutant' / mutant_receipts[mutant['id']]['object']).read_bytes())
            with self.assertRaises(ValueError):
                compiled_data(data_owner, manifest['regions'], {mutant['id']: mutant_module}, MZ.parse(original))
