"""Fresh F_2269 comparison and a stride-layout negative control.

Converted to use the canonical prover (tools/probe_module.py) via
tests/support_probe.py so this test no longer depends on exact whitespace
or declaration placement in the production sources.
"""
from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
sys.path.insert(0, str(Path(__file__).resolve().parent))
from reconstruct import read_json
from support_probe import exact, mutant_rejected


class MatchingCWave29Tests(unittest.TestCase):
    def test_fresh_match_and_stride_mutant(self):
        owner = read_json(ROOT / 'recipes/c/matching-wave29.json')['owners'][0]
        exact(owner['id'])
        mutant_rejected(owner['id'],
                        b'gfx_copy_rect(g0736, g0738, resource_stripe_table + g072e * 0x2a2, g073a);',
                        b'gfx_copy_rect(g0736, g0738, resource_stripe_table + g072e * 0x2a1, g073a);')
