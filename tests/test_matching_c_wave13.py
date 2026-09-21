"""Full fresh comparisons and expression/layout mutants for the thirteenth C wave.

The fresh-wave/mutant part below is converted to use the canonical prover
(tools/probe_module.py) via tests/support_probe.py; the storage evidence
test asserts a different artifact and is left unchanged.
"""
import copy
from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
sys.path.insert(0, str(Path(__file__).resolve().parent))
from storage_evidence import verify, verify_bindings
from reconstruct import read_json
from support_probe import exact, mutant_rejected


class MatchingCWave13Tests(unittest.TestCase):
    def test_fresh_wave_and_expression_layout_mutants(self):
        owners = read_json(ROOT / 'recipes/c/matching-wave13.json')['owners']
        for owner in owners:
            exact(owner['id'])
        for name, before, after in (
            ('F_B772', b'i<=11', b'i<=10'),):
            mutant_rejected(name, before, after)

    def test_storage_evidence_rejects_changed_address_site_and_missing_access(self):
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        manifest = read_json(ROOT / 'layout/manifest.json')
        evidence = read_json(ROOT / 'docs/storage-binding-evidence.json')
        verified = verify(evidence, original, manifest['frames'])
        self.assertEqual({k: verified[k] for k in ('DS_C588', 'DS_C5A2')},
                         {'DS_C588': 0xc588, 'DS_C5A2': 0xc5a2})
        for variant in ('address', 'site', 'missing', 'extent', 'identity'):
            changed = copy.deepcopy(evidence)
            item = changed['objects'][0]
            if variant == 'address':
                item['offset'] += 2
            elif variant == 'site':
                item['observations'][0]['load_offset'] += 1
            elif variant == 'missing':
                item['observations'].pop()
            elif variant == 'extent':
                item['observations'][0]['function_extent']['end'] -= 1
            else:
                changed['original_sha256'] = '0' * 64
            with self.subTest(variant=variant), self.assertRaises(ValueError):
                verify(changed, original, manifest['frames'])
        changed = copy.deepcopy(manifest)
        owner = next(o for o in changed['regions'] if o['id'] == 'F_B772')
        owner['build']['bindings']['_gc588']['offset'] += 2
        with self.assertRaisesRegex(ValueError, 'Binding contradicts'):
            verify_bindings(ROOT, changed, original)
