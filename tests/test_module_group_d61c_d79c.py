"""Fresh shared-compilation proof for the retired D61C..D79C C candidate.

recovery/recipes/modules/C_D61C_D79C.json is the C-candidate recipe for the
draw-queue renderers, whose production owner is the hand-written
asm/M_D61C_D79C.ASM (push di/push si hand order, mov bp,sp argument patching;
docs/current/asm-provenance.json).  The retired candidate still reproduces the
original bytes; the check bypasses probe_module_group's manifest-ownership
guard exactly as tests/test_module_group_c5d1_c898.py does.
"""
from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tests'))

from test_module_group_c5d1_c898 import probe_retired_c_candidate


class D61CD79CModuleGroupTests(unittest.TestCase):
    def test_contiguous_decoder_pair_has_one_exact_object(self):
        report = probe_retired_c_candidate(ROOT / 'recovery/recipes/modules/C_D61C_D79C.json')
        self.assertEqual(report['status'], 'EQUAL')
        self.assertEqual((report['source_units_combined'], report['text_bytes'],
                          report['fixups_checked']), (2, 507, 0))


if __name__ == '__main__':
    unittest.main()
