"""Fresh F_21DB comparison and DS:96EE buffer evidence checks.

The fresh-match/mutant part below is converted to use the canonical prover
(tools/probe_module.py) via tests/support_probe.py; the buffer storage
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
from support_probe import exact, mutant_rejected


class MatchingCWave28Tests(unittest.TestCase):
    def test_fresh_match_and_loop_bound_mutant(self):
        owner = read_json(ROOT / 'recipes/c/matching-wave28.json')['owners'][0]
        exact(owner['id'])
        mutant_rejected(owner['id'], b'i < 23', b'i < 22')

    def test_buffer_storage_evidence_is_independent(self):
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        manifest = read_json(ROOT / 'layout/manifest.json')
        document = read_json(ROOT / 'docs/storage-binding-evidence.json')
        self.assertEqual(verify(document, original, manifest['frames'])['DS_96EE'], 0x96EE)
        item = next(o for o in document['objects'] if o['id'] == 'DS_96EE')
        for variant in ('address', 'site', 'access', 'extent', 'missing'):
            changed = copy.deepcopy(document)
            changed_item = next(o for o in changed['objects'] if o['id'] == 'DS_96EE')
            first = changed_item['observations'][0]
            if variant == 'address': changed_item['offset'] += 1
            elif variant == 'site': first['load_offset'] += 1
            elif variant == 'access': first['access'] = 'read'
            elif variant == 'extent': first['function_extent']['end'] -= 1
            else: changed_item['observations'].pop()
            with self.subTest(variant=variant), self.assertRaises(ValueError):
                verify(changed, original, manifest['frames'])
