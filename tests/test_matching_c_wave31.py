"""Fresh critical-error handler pair and byte-storage evidence checks.

The fresh-match part below is converted to use the canonical prover
(tools/probe_module.py) via tests/support_probe.py; the byte storage
evidence test asserts a different artifact and is left unchanged.
"""
import copy
from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
sys.path.insert(0, str(Path(__file__).resolve().parent))
from reconstruct import read_json
from storage_evidence import verify
from support_probe import exact


class MatchingCWave31Tests(unittest.TestCase):
    def test_fresh_handler_pair(self):
        owners = read_json(ROOT / 'recipes/c/matching-wave31.json')['owners']
        for owner in owners:
            exact(owner['id'])

    def test_byte_storage_evidence_controls(self):
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        manifest = read_json(ROOT / 'layout/manifest.json')
        document = read_json(ROOT / 'docs/storage-binding-evidence.json')
        self.assertEqual(verify(document, original, manifest['frames'])['DS_C0C8'], 0xC0C8)
        changed = copy.deepcopy(document)
        item = next(o for o in changed['objects'] if o['id'] == 'DS_C0C8')
        item['observations'][1]['load_offset'] += 1
        with self.assertRaises(ValueError):
            verify(changed, original, manifest['frames'])
