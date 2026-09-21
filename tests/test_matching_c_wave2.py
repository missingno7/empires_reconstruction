"""Full fresh comparisons and expression/layout mutants for the second C wave.

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


class MatchingCWave2Tests(unittest.TestCase):
    def test_fresh_wave_and_expression_layout_mutants(self):
        owners = read_json(ROOT / 'recipes/c/matching-wave2.json')['owners']
        for owner in owners:
            exact(owner['id'])
        for name, before, after in (
            ('F_AF45', b'if (++g13ef[0] >= 3)', b'g13ef[0]++; if (g13ef[0] >= 3)'),
            ('F_7417', b'pad[22]', b'pad[24]')):
            mutant_rejected(name, before, after)
