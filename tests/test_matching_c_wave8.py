"""Full fresh comparisons and expression/layout mutants for the eighth C wave.

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


class MatchingCWave8Tests(unittest.TestCase):
    def test_fresh_wave_and_expression_layout_mutants(self):
        owners = read_json(ROOT / 'recipes/c/matching-wave8.json')['owners']
        for owner in owners:
            exact(owner['id'])
        for name, before, after in (
            ('F_32FA', b'register int i;int j;', b'register int i,j;'),
            # F_2A70 was normalized to return an unsigned char near
            # pointer instead of an int cast; the +1 displacement it
            # mutates is unchanged.
            ('F_2A70', b'return (unsigned char near *)(p+*p*3+1);',
             b'return (unsigned char near *)(p+*p*3+2);')):
            mutant_rejected(name, before, after)
