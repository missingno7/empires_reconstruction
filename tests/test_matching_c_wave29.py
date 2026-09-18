"""Fresh F_2269 comparison and a stride-layout negative control."""
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


class MatchingCWave29Tests(unittest.TestCase):
    def test_fresh_match_and_stride_mutant(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        owner = read_json(ROOT / 'recipes/c/matching-wave29.json')['owners'][0]
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        lock = read_json(ROOT / 'layout/toolchain.json')
        modules = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            work = Path(temporary)
            source = (ROOT / owner['source']).read_bytes()
            mutant_path = work / 'F_2269_MUTANT.C'
            mutant_path.write_bytes(source.replace(b'0x2a2', b'0x2a1'))
            mutant = copy.deepcopy(owner)
            mutant.update(id='F_2269_MUTANT', source=mutant_path.relative_to(ROOT).as_posix())
            receipts, _ = compile_sources(ROOT, [owner, mutant], work / 'compiler', ROOT / 'toolchain',
                                          Path(lock['dosbox_default']), lock)
            module = read_object((work / 'compiler' / receipts[owner['id']]['object']).read_bytes())
            data, _ = bind_region(owner, module, MZ.parse(original), manifest['frames'], manifest['regions'], modules)
            mismatch(original[owner['start']:owner['end']], data, owner)
            mutant_module = read_object((work / 'compiler' / receipts[mutant['id']]['object']).read_bytes())
            with self.assertRaises(ValueError):
                mutant_data, _ = bind_region(mutant, mutant_module, MZ.parse(original), manifest['frames'], manifest['regions'], modules)
                mismatch(original[mutant['start']:mutant['end']], mutant_data, mutant)
