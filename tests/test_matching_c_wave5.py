"""Full fresh comparisons and expression/layout mutants for the fifth C wave.

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


class MatchingCWave5Tests(unittest.TestCase):
    def test_fresh_wave_and_expression_layout_mutants(self):
        owners = read_json(ROOT / 'recipes/c/matching-wave5.json')['owners']
        for owner in owners:
            exact(owner['id'])
        for name, before, after in (
            # F_7D91 was normalized to use the shared DIALOG.H struct/field
            # names; the equivalent layout mutant now reorders the two
            # independent field stores instead of chaining the old raw
            # struct's zero-valued assignments (see include/DIALOG.H).
            ('F_7D91', b'q.title = 0;\n    q.sub = 1;', b'q.sub = 1;\n    q.title = 0;'),
            ('F_963E',
             b'puzzle_clear_cell(a,b) int a; register int b; {register int x; int y;if(b<3)',
             b'puzzle_clear_cell(a,b) int a; register int b; {register int x; int y;if(b<4)')):
            mutant_rejected(name, before, after)
