"""Full fresh comparisons and expression/layout mutants for the sixteenth C wave.

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


class MatchingCWave16Tests(unittest.TestCase):
    def test_fresh_wave_and_expression_layout_mutants(self):
        owners = read_json(ROOT / 'recipes/c/matching-wave16.json')['owners']
        for owner in owners:
            exact(owner['id'])
        for name, before, after in (
            ('F_1D47', b'i<23', b'i<22'),
            ('F_B4FB', b'g40d4[72]=3', b'g40d4[73]=3')):
            mutant_rejected(name, before, after)
