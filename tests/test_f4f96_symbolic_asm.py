import tempfile
import unittest
from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))

from mz import MZ
from dos_runner import resolve_runner
from reconstruct import (bind_region, compile_sources, mismatch,
                         owned_library_modules, read_json, read_object)


class F4F96SymbolicAssemblyTests(unittest.TestCase):
    def test_symbolic_option_parser_is_exact_and_nonrelocating(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        owner = next(item for item in manifest['regions'] if item['id'] == 'F_4F96')
        # F_4F96 was recovered as exact C; see docs/current/exact-c-recovery.md.
        # It was later folded into the src/STARTUP.C translation-unit merge
        # (module C_4F63_520A in layout/production-plan.json; see
        # docs/current/asm-provenance.json), so its old standalone
        # src/CMDLINE.C is gone.
        self.assertEqual((owner['kind'], owner['source']),
                         ('MATCHING_C', 'src/STARTUP.C'))
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        lock = read_json(ROOT / 'layout/toolchain.json')
        libraries = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            receipts, _ = compile_sources(ROOT, [owner], Path(temporary), ROOT / 'toolchain',
                                           resolve_runner(lock), lock)
            module = read_object((Path(temporary) / receipts['F_4F96']['object']).read_bytes())
            data, proof = bind_region(owner, module, MZ.parse(original), manifest['frames'],
                                      manifest['regions'], libraries)
        # The compiled module now covers the whole merged src/STARTUP.C unit
        # (six functions), not just F_4F96, so module.publics/module.fixups
        # cover all of them; check this owner's own public/fixups instead.
        self.assertIn('_cmdline_parse_args', [p['name'] for p in module.publics])
        self.assertEqual(len(module.publics), 6)
        # The C recovery references argv/argc and helper globals by external
        # fixup and uses a switch-case jump table (local _TEXT segment
        # fixups) instead of the ASM version's purely local addressing; no
        # fixup targets outside those two forms, and none survive as a
        # load-time relocation (checked below). Scoped to F_4F96's own
        # extent, the fixup count is unchanged from before the merge.
        self.assertEqual(len(proof['fixups']), 36)
        self.assertTrue(all(f['target_kind'] in ('external', 'segment') for f in proof['fixups']))
        mismatch(original[owner['start']:owner['end']], data, owner)
        self.assertEqual(len(data), 299)
        self.assertEqual(proof['load_relocations'], [])


if __name__ == '__main__':
    unittest.main()
