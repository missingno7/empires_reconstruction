"""Converted to use the canonical prover (tools/probe_module.py) via
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


class MatchingCWave27Tests(unittest.TestCase):
    def test_fresh_match_and_switch_order_mutant(self):
        owner = read_json(ROOT / 'recipes/c/matching-wave27.json')['owners'][0]
        exact(owner['id'])
        mutant_rejected(
            owner['id'],
            b'case 27: di = 1; i = 0; break;\n        case 328:\n        case 336: i ^= 1; quit_confirm_toggle_draw(); break;\n        case 13: di = i; break;',
            b'case 13: di = i; break;\n        case 27: di = 1; i = 0; break;\n        case 328:\n        case 336: i ^= 1; quit_confirm_toggle_draw(); break;')
