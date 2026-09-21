"""Full fresh comparisons and expression/layout mutants for the fifteenth C wave."""
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


class MatchingCWave15Tests(unittest.TestCase):
    def test_fresh_wave_and_expression_layout_mutants(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        owners = read_json(ROOT / 'recipes/c/matching-wave15.json')['owners']
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        lock = read_json(ROOT / 'layout/toolchain.json')
        modules = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            work = Path(temporary)
            mutants = []
            for name, before, after in (
                ('F_D3DA', b'+2;', b'+3;'),
                # F_C0E0 was normalized to use the shared R3E8.H record table
                # declared as an unsized extern array; mutate the record index
                # literal instead of the old fixed array-size literal.
                ('F_C0E0', b'board_record_index=1', b'board_record_index=2'),
                ('F_C15E', b'hud_scroll_move(-3)', b'hud_scroll_move(-2)')):
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
                if owner['id'] == 'F_D3DA':
                    self.assertEqual(owner['end'], 512 + 0xd45c)
                    self.assertEqual(data[-6:], bytes.fromhex('5f 5e 8b e5 5d c3'))
                    truncated = copy.deepcopy(owner)
                    truncated['end'] = truncated['start'] + 105
                    with self.assertRaises(ValueError):
                        bind_region(truncated, module, MZ.parse(original), manifest['frames'], manifest['regions'], modules)
            self.assertEqual(checked, 322)
            for owner in mutants:
                module = read_object((work / 'compiler' / receipts[owner['id']]['object']).read_bytes())
                with self.assertRaises(ValueError):
                    data, _ = bind_region(owner, module, MZ.parse(original), manifest['frames'], manifest['regions'], modules)
                    mismatch(original[owner['start']:owner['end']], data, owner)
