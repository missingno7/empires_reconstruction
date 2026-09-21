"""Fresh F_2AE2 compile and mutation control."""
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


class MatchingCWave34Tests(unittest.TestCase):
    def _bound(self, owner, source, work, original, manifest, lock, modules):
        candidate = dict(owner)
        candidate['source'] = source
        receipts, _ = compile_sources(ROOT, [candidate], work, ROOT / 'toolchain',
                                      resolve_runner(lock), lock)
        module = read_object((work / receipts['F_2AE2']['object']).read_bytes())
        data, _ = bind_region(candidate, module, MZ.parse(original), manifest['frames'],
                              manifest['regions'], modules)
        return data

    def test_fresh_redraw_matches(self):
        owner = read_json(ROOT / 'recipes/c/matching-wave34.json')['owners'][0]
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        lock = read_json(ROOT / 'layout/toolchain.json')
        manifest = read_json(ROOT / 'layout/manifest.json')
        modules = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            data = self._bound(owner, owner['source'], Path(temporary) / 'compiler',
                               original, manifest, lock, modules)
            mismatch(original[owner['start']:owner['end']], data, owner)

    def test_source_mutation_changes_bound_bytes(self):
        owner = read_json(ROOT / 'recipes/c/matching-wave34.json')['owners'][0]
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        lock = read_json(ROOT / 'layout/toolchain.json')
        manifest = read_json(ROOT / 'layout/manifest.json')
        modules = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            mutated = 'build/F_2AE2_mutated.C'
            source = (ROOT / owner['source']).read_text().replace('0x2a', '0x2b', 1)
            (ROOT / mutated).write_text(source)
            try:
                data = self._bound(owner, mutated, Path(temporary) / 'compiler',
                                   original, manifest, lock, modules)
                self.assertNotEqual(original[owner['start']:owner['end']], data)
            finally:
                (ROOT / mutated).unlink(missing_ok=True)
