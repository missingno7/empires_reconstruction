"""Full fresh comparisons and expression/layout mutants for the eighteenth C wave.

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


class MatchingCWave18Tests(unittest.TestCase):
    def test_fresh_wave_and_expression_layout_mutants(self):
        owners = read_json(ROOT / 'recipes/c/matching-wave18.json')['owners']
        for owner in owners:
            exact(owner['id'])
        for name, before, after in (
            ('F_929E', b'int a,b;{int u,v;register int x,y;', b'int a,b;{int v,u;register int x,y;'),
            ('F_950C', b'rect_border_draw(--x,--y,w,h)', b'rect_border_draw(x--,y--,w,h)')):
            mutant_rejected(name, before, after)
