"""Fresh F_338A compile and mutation control."""
from pathlib import Path
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from reconstruct import (read_json, compile_sources, read_object, bind_region,
                         mismatch, owned_library_modules)
from mz import MZ


class MatchingCWave33Tests(unittest.TestCase):
    def test_fresh_dispatcher_matches(self):
        recipe = read_json(ROOT / 'recipes/c/matching-wave33.json')
        owner = recipe['owners'][0]
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        lock = read_json(ROOT / 'layout/toolchain.json')
        manifest = read_json(ROOT / 'layout/manifest.json')
        modules = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            work = Path(temporary)
            receipts, _ = compile_sources(ROOT, [owner], work / 'compiler', ROOT / 'toolchain',
                                           Path(lock['dosbox_default']), lock)
            module = read_object((work / 'compiler' / receipts['F_338A']['object']).read_bytes())
            data, _ = bind_region(owner, module, MZ.parse(original), manifest['frames'],
                                  manifest['regions'], modules)
            mismatch(original[owner['start']:owner['end']], data, owner)

    def test_source_mutation_changes_bound_bytes(self):
        recipe = read_json(ROOT / 'recipes/c/matching-wave33.json')
        owner = recipe['owners'][0]
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        lock = read_json(ROOT / 'layout/toolchain.json')
        manifest = read_json(ROOT / 'layout/manifest.json')
        modules = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            work = Path(temporary)
            mutated = dict(owner)
            mutated['source'] = 'build/F_338A_mutated.C'
            source = (ROOT / owner['source']).read_text().replace('0x2ac', '0x2ad', 1)
            (ROOT / mutated['source']).write_text(source)
            try:
                receipts, _ = compile_sources(ROOT, [mutated], work / 'compiler', ROOT / 'toolchain',
                                               Path(lock['dosbox_default']), lock)
                module = read_object((work / 'compiler' / receipts['F_338A']['object']).read_bytes())
                data, _ = bind_region(mutated, module, MZ.parse(original), manifest['frames'],
                                      manifest['regions'], modules)
                self.assertNotEqual(original[owner['start']:owner['end']], data)
            finally:
                (ROOT / mutated['source']).unlink(missing_ok=True)
