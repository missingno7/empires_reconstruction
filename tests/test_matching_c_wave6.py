"""Full fresh comparisons and expression/layout mutants for the sixth C wave.

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


class MatchingCWave6Tests(unittest.TestCase):
    def test_fresh_wave_and_expression_layout_mutants(self):
        owners = read_json(ROOT / 'recipes/c/matching-wave6.json')['owners']
        for owner in owners:
            exact(owner['id'])
        for name, before, after in (
            ('F_67DC', b'unsigned *t;char *q;', b'char *q;unsigned *t;'),
            ('F_D15D', b'case 0x150:', b'case 0x151:')):
            mutant_rejected(name, before, after)
