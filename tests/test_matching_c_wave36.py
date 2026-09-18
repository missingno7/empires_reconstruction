"""Fresh F_A85E compile and pooled-string mutation control."""
from pathlib import Path
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from reconstruct import (read_json, compile_sources, read_object, bind_region,
                         mismatch, owned_library_modules)
from mz import MZ


class MatchingCWave36Tests(unittest.TestCase):
    def test_fresh_selection_routine_matches(self):
        owner = read_json(ROOT / 'recipes/c/matching-wave36.json')['owners'][0]
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        lock = read_json(ROOT / 'layout/toolchain.json')
        manifest = read_json(ROOT / 'layout/manifest.json')
        modules = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            receipts, _ = compile_sources(ROOT, [owner], Path(temporary) / 'compiler',
                                           ROOT / 'toolchain', Path(lock['dosbox_default']), lock)
            module = read_object((Path(temporary) / 'compiler' /
                                  receipts['F_A85E']['object']).read_bytes())
            data, _ = bind_region(owner, module, MZ.parse(original), manifest['frames'],
                                  manifest['regions'], modules)
            mismatch(original[owner['start']:owner['end']], data, owner)

    def test_pooled_string_binding_mutation_changes_bytes(self):
        owner = read_json(ROOT / 'recipes/c/matching-wave36.json')['owners'][0]
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        lock = read_json(ROOT / 'layout/toolchain.json')
        manifest = read_json(ROOT / 'layout/manifest.json')
        modules = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            mutated = 'build/F_A85E_mutated.C'
            source = (ROOT / owner['source']).read_text().replace('0x58', '0x59', 1)
            (ROOT / mutated).write_text(source)
            try:
                candidate = dict(owner)
                candidate['source'] = mutated
                receipts, _ = compile_sources(ROOT, [candidate], Path(temporary) / 'compiler',
                                               ROOT / 'toolchain', Path(lock['dosbox_default']), lock)
                module = read_object((Path(temporary) / 'compiler' /
                                      receipts['F_A85E']['object']).read_bytes())
                data, _ = bind_region(candidate, module, MZ.parse(original), manifest['frames'],
                                      manifest['regions'], modules)
                self.assertNotEqual(original[owner['start']:owner['end']], data)
            finally:
                (ROOT / mutated).unlink(missing_ok=True)
