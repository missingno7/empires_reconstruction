"""Full fresh comparisons and expression/layout mutants for the twenty-fifth C wave.

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


class MatchingCWave25Tests(unittest.TestCase):
    def test_fresh_wave_and_expression_layout_mutants(self):
        owners = read_json(ROOT / 'recipes/c/matching-wave25.json')['owners']
        for owner in owners:
            exact(owner['id'])
        for name, before, after in (
            # The double-cast mutation this test used to exercise now
            # compiles identically to its target expression (the
            # refactor's normalized near/far cast chain optimizes the
            # same way either form is written), so it no longer produces
            # a mismatch; mutate the neighbouring width adjustment
            # instead, which still changes emitted layout bytes.
            ('F_8480', b'w-=4;', b'w-=3;'),
            ('F_7BFC', b'char pad[12]', b'char pad[10]')):
            mutant_rejected(name, before, after)
