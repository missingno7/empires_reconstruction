"""Full fresh comparisons and expression/layout mutants for the twenty-fourth C wave."""
import copy
from pathlib import Path
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from mz import MZ
from reconstruct import (read_json, compile_sources, read_object, bind_region, mismatch,
                         owned_library_modules)


class MatchingCWave24Tests(unittest.TestCase):
    def test_fresh_wave_and_expression_layout_mutants(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        owners = read_json(ROOT / 'recipes/c/matching-wave24.json')['owners']
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        lock = read_json(ROOT / 'layout/toolchain.json')
        modules = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            work = Path(temporary)
            mutants = []
            for name, before, after in (
                ('F_90A6', b'((int *)gc5c6+2)[k]', b'((int *)gc5c6)[k+2]'),
                ('F_969D', b'gc316[i][j].b!=3', b'gc316[i][j].b!=2')):
                owner = copy.deepcopy(next(o for o in owners if o['id'] == name))
                source = (ROOT / owner['source']).read_bytes()
                self.assertEqual(source.count(before), 1)
                path = work / (name + '.C')
                path.write_bytes(source.replace(before, after))
                owner.update(id=name + '_MUTANT', source=path.relative_to(ROOT).as_posix())
                mutants.append(owner)
            receipts, _ = compile_sources(ROOT, owners + mutants, work / 'compiler', ROOT / 'toolchain',
                                          Path(lock['dosbox_default']), lock)
            checked = 0
            for owner in owners:
                module = read_object((work / 'compiler' / receipts[owner['id']]['object']).read_bytes())
                data, _ = bind_region(owner, module, MZ.parse(original), manifest['frames'], manifest['regions'], modules)
                mismatch(original[owner['start']:owner['end']], data, owner)
                checked += len(data)
            self.assertEqual(checked, 866)
            for owner in mutants:
                module = read_object((work / 'compiler' / receipts[owner['id']]['object']).read_bytes())
                with self.assertRaises(ValueError):
                    data, _ = bind_region(owner, module, MZ.parse(original), manifest['frames'], manifest['regions'], modules)
                    mismatch(original[owner['start']:owner['end']], data, owner)
