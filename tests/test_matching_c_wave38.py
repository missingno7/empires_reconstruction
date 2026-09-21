"""Fresh F_21A9/F_233E compile and mutation controls."""
from pathlib import Path
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from dos_runner import resolve_runner
from reconstruct import (read_json, compile_sources, read_object, bind_region,
                         mismatch, owned_library_modules)
from mz import MZ


class MatchingCWave38Tests(unittest.TestCase):
    def test_fresh_small_routines_match(self):
        recipe = read_json(ROOT / 'recipes/c/matching-wave38.json')
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        lock = read_json(ROOT / 'layout/toolchain.json')
        manifest = read_json(ROOT / 'layout/manifest.json')
        modules = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            for owner in recipe['owners']:
                work = Path(temporary) / owner['id']
                receipts, _ = compile_sources(ROOT, [owner], work, ROOT / 'toolchain',
                                               resolve_runner(lock), lock)
                module = read_object((work / receipts[owner['id']]['object']).read_bytes())
                data, _ = bind_region(owner, module, MZ.parse(original), manifest['frames'],
                                      manifest['regions'], modules)
                mismatch(original[owner['start']:owner['end']], data, owner)

    def test_f233e_mutation_changes_bound_bytes(self):
        recipe = read_json(ROOT / 'recipes/c/matching-wave38.json')
        owner = next(o for o in recipe['owners'] if o['id'] == 'F_233E')
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        lock = read_json(ROOT / 'layout/toolchain.json')
        manifest = read_json(ROOT / 'layout/manifest.json')
        modules = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            mutated = 'build/F_233E_mutated.C'
            source = (ROOT / owner['source']).read_text().replace('0x0d', '0x0e', 1)
            (ROOT / mutated).write_text(source)
            try:
                candidate = dict(owner)
                candidate['source'] = mutated
                work = Path(temporary) / 'mutated'
                receipts, _ = compile_sources(ROOT, [candidate], work, ROOT / 'toolchain',
                                               resolve_runner(lock), lock)
                module = read_object((work / receipts['F_233E']['object']).read_bytes())
                data, _ = bind_region(candidate, module, MZ.parse(original), manifest['frames'],
                                      manifest['regions'], modules)
                self.assertNotEqual(original[owner['start']:owner['end']], data)
            finally:
                (ROOT / mutated).unlink(missing_ok=True)
