"""Fresh F_3A75 compile and mutation control."""
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


class MatchingCWave37Tests(unittest.TestCase):
    # F_3A75 (the main turn loop) was folded into the src/GAME.C
    # translation-unit merge (module C_3A75_4A93 in
    # layout/production-plan.json; see docs/current/asm-provenance.json);
    # its old standalone src/TURNLOOP.C is gone. It no longer needs the
    # invented -B flag either -- the merged unit's own inline asm now
    # explains the frame -- so recipes/c/matching-wave37.json is updated to
    # compile it plainly from src/GAME.C.
    def test_fresh_turn_loop_matches(self):
        owner = read_json(ROOT / 'recipes/c/matching-wave37.json')['owners'][0]
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        lock = read_json(ROOT / 'layout/toolchain.json')
        manifest = read_json(ROOT / 'layout/manifest.json')
        modules = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            receipts, _ = compile_sources(ROOT, [owner], Path(temporary) / 'compiler',
                                           ROOT / 'toolchain', resolve_runner(lock), lock)
            module = read_object((Path(temporary) / 'compiler' /
                                  receipts['F_3A75']['object']).read_bytes())
            data, _ = bind_region(owner, module, MZ.parse(original), manifest['frames'],
                                  manifest['regions'], modules)
            mismatch(original[owner['start']:owner['end']], data, owner)

    def test_source_mutation_changes_bound_bytes(self):
        owner = read_json(ROOT / 'recipes/c/matching-wave37.json')['owners'][0]
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        lock = read_json(ROOT / 'layout/toolchain.json')
        manifest = read_json(ROOT / 'layout/manifest.json')
        modules = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            mutated = 'build/F_3A75_mutated.C'
            # Plain '0x18' is no longer unique to F_3A75 now that the merged
            # src/GAME.C also carries an unrelated '0x18' inside a comment
            # elsewhere in the file. Anchor the replacement to the call that
            # is unique to F_3A75's body.
            before, after = 'timer_deadline_arm(0x18)', 'timer_deadline_arm(0x19)'
            text = (ROOT / owner['source']).read_text()
            self.assertEqual(text.count(before), 1)
            source = text.replace(before, after, 1)
            (ROOT / mutated).write_text(source)
            try:
                candidate = dict(owner)
                candidate['source'] = mutated
                receipts, _ = compile_sources(ROOT, [candidate], Path(temporary) / 'compiler',
                                               ROOT / 'toolchain', resolve_runner(lock), lock)
                module = read_object((Path(temporary) / 'compiler' /
                                      receipts['F_3A75']['object']).read_bytes())
                data, _ = bind_region(candidate, module, MZ.parse(original), manifest['frames'],
                                      manifest['regions'], modules)
                self.assertNotEqual(original[owner['start']:owner['end']], data)
            finally:
                (ROOT / mutated).unlink(missing_ok=True)
