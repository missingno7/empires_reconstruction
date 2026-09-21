"""Fresh complete-owner proofs for the recovered C470 record header."""
from pathlib import Path
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))

from mz import MZ
from dos_runner import resolve_runner
from reconstruct import (bind_region, compile_sources, mismatch, owned_library_modules,
                         read_json, read_object)


class C470RecordHeaderTests(unittest.TestCase):
    def test_header_users_preserve_code_and_fixups(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        lock = read_json(ROOT / 'layout/toolchain.json')
        ids = ('F_9DCC', 'F_A09D', 'F_A13F', 'F_A223', 'F_A24E', 'F_A28D', 'F_A33F',
               'F_A036')
        owners = [next(item for item in manifest['regions'] if item['id'] == ident)
                  for ident in ids]
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            work = Path(temporary)
            receipts, _ = compile_sources(ROOT, owners, work, ROOT / 'toolchain',
                                            resolve_runner(lock), lock)
            modules = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
            for owner in owners:
                module = read_object((work / receipts[owner['id']]['object']).read_bytes())
                data, _ = bind_region(owner, module, MZ.parse(original), manifest['frames'],
                                      manifest['regions'], modules)
                mismatch(original[owner['start']:owner['end']], data, owner)


if __name__ == '__main__':
    unittest.main()
