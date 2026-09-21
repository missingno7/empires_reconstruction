"""Full fresh comparisons and expression/layout mutants for the twenty-fifth C wave."""
import copy
from pathlib import Path
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from mz import MZ
from dos_runner import resolve_runner
from reconstruct import (read_json, compile_sources, read_object, bind_region, mismatch,
                         owned_library_modules)


class MatchingCWave25Tests(unittest.TestCase):
    def test_fresh_wave_and_expression_layout_mutants(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        owners = read_json(ROOT / 'recipes/c/matching-wave25.json')['owners']
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        lock = read_json(ROOT / 'layout/toolchain.json')
        modules = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            work = Path(temporary)
            mutants = []
            for name, before, after in (
                # The double-cast mutation this test used to exercise now
                # compiles identically to its target expression (the
                # refactor's normalized near/far cast chain optimizes the
                # same way either form is written), so it no longer produces
                # a mismatch; mutate the neighbouring width adjustment
                # instead, which still changes emitted layout bytes.
                ('F_8480', b'w-=4;', b'w-=3;'),
                ('F_7BFC', b'char pad[12]', b'char pad[10]')):
                owner = copy.deepcopy(next(o for o in owners if o['id'] == name))
                source = (ROOT / owner['source']).read_bytes()
                self.assertEqual(source.count(before), 1)
                path = work / (name + '.C')
                path.write_bytes(source.replace(before, after))
                owner.update(id=name + '_MUTANT', source=path.relative_to(ROOT).as_posix())
                mutants.append(owner)
            receipts, _ = compile_sources(ROOT, owners + mutants, work / 'compiler', ROOT / 'toolchain',
                                          resolve_runner(lock), lock)
            checked = 0
            for owner in owners:
                module = read_object((work / 'compiler' / receipts[owner['id']]['object']).read_bytes())
                data, _ = bind_region(owner, module, MZ.parse(original), manifest['frames'], manifest['regions'], modules)
                mismatch(original[owner['start']:owner['end']], data, owner)
                checked += len(data)
            self.assertEqual(checked, 990)
            for owner in mutants:
                module = read_object((work / 'compiler' / receipts[owner['id']]['object']).read_bytes())
                with self.assertRaises(ValueError):
                    data, _ = bind_region(owner, module, MZ.parse(original), manifest['frames'], manifest['regions'], modules)
                    mismatch(original[owner['start']:owner['end']], data, owner)
