"""Full fresh comparisons and expression/layout mutants for the fourteenth C wave.

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


class MatchingCWave14Tests(unittest.TestCase):
    def test_fresh_wave_and_expression_layout_mutants(self):
        owners = read_json(ROOT / 'recipes/c/matching-wave14.json')['owners']
        for owner in owners:
            exact(owner['id'])
        for name, before, after in (
            ('F_CF3C', b'n <<= 2;\n        n += i;\n        n += 3;', b'n = n * 4 + i + 3;'),
            ('F_CE9E', b'for (i = 0; i < 4; i++) {\n        x = g22e0[i]', b'for (i = 0; i < 3; i++) {\n        x = g22e0[i]')):
            mutant_rejected(name, before, after)
