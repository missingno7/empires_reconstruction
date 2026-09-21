"""Fresh proof for the former F_7DD3 cleanup continuation, now part of F_7964."""
from pathlib import Path
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from mz import MZ
from dos_runner import resolve_runner
from reconstruct import (read_json, compile_sources, read_object, bind_region,
                         mismatch, owned_library_modules)


class SymbolicAsmWave149Tests(unittest.TestCase):
    def test_f_7dd3_has_complete_extent_and_real_call_fixups(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        # F_7DD3 was recovered as exact C and absorbed into F_7964's shared
        # frame (see docs/history/exact-c-recovery.md, "F_7964 and former
        # F_7DD3: exact menu loop"); it no longer exists as its own manifest
        # region. F_7964's provenance.former_regions still records F_7DD3's
        # original [32211, 32252) extent (41 bytes), which this test verifies
        # directly within the freshly compiled and bound F_7964 owner.
        owner = next(region for region in manifest['regions'] if region['id'] == 'F_7964')
        self.assertEqual(owner['kind'], 'MATCHING_C')
        self.assertEqual(owner['source'], 'src/MENULOOP.C')
        former = next(r for r in owner['provenance']['former_regions'] if r['id'] == 'F_7DD3')
        self.assertEqual(former['end'] - former['start'], 41)

        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        lock = read_json(ROOT / 'layout/toolchain.json')
        modules = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            receipts, _ = compile_sources(ROOT, [owner], Path(temporary), ROOT / 'toolchain',
                                          resolve_runner(lock), lock)
            module = read_object((Path(temporary) / receipts[owner['id']]['object']).read_bytes())
            data, proof = bind_region(owner, module, MZ.parse(original), manifest['frames'],
                                      manifest['regions'], modules)
        mismatch(original[owner['start']:owner['end']], data, owner)

        sub_start, sub_end = former['start'] - owner['start'], former['end'] - owner['start']
        self.assertEqual(data[sub_start:sub_end], original[former['start']:former['end']])
        sub_fixups = [(fixup['extent_offset'] - sub_start, fixup['target']) for fixup in proof['fixups']
                     if sub_start <= fixup['extent_offset'] < sub_end]
        self.assertEqual(sub_fixups,
                         [(5, '_menu_list_draw'), (15, '_menu_list_enable'), (18, '_sound_request_count_dec'),
                          (21, '_ui_overlay_hide'), (30, '_keyboard_chain_disable'), (33, '_keyboard_buffer_drain')])
        self.assertEqual(proof['load_relocations'], [])


if __name__ == '__main__':
    unittest.main()
