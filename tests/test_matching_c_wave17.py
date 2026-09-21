"""Full fresh comparisons and expression/layout mutants for the seventeenth C wave.

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


class MatchingCWave17Tests(unittest.TestCase):
    def test_fresh_wave_and_expression_layout_mutants(self):
        owners = read_json(ROOT / 'recipes/c/matching-wave17.json')['owners']
        for owner in owners:
            exact(owner['id'])
        for name, before, after in (
            ('F_703E', b'case 5:\n        gfx_color_select(0);\n        rect_border_draw',
             b'case 4:\n        gfx_color_select(0);\n        rect_border_draw'),):
            mutant_rejected(name, before, after)
