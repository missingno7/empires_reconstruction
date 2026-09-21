"""Full fresh comparisons and expression/layout mutants for the twenty-first C wave.

The fresh-wave/mutant part below is converted to use the canonical prover
(tools/probe_module.py) via tests/support_probe.py; the indexed-base
evidence test asserts a different artifact and is left unchanged.
"""
import copy
from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
sys.path.insert(0, str(Path(__file__).resolve().parent))
from storage_evidence import verify, verify_bindings
import hashlib
from reconstruct import read_json
from support_probe import exact, mutant_rejected


class MatchingCWave21Tests(unittest.TestCase):
    def test_fresh_wave_and_expression_layout_mutants(self):
        owners = read_json(ROOT / 'recipes/c/matching-wave21.json')['owners']
        for owner in owners:
            exact(owner['id'])
        for name, before, after in (
            # F_AD25 was normalized to the shared C470.H record's 'text'
            # field name instead of the old raw struct's 'a' field.
            ('F_AD25',
             b'void fad25()\n{\n    register int i;\n\n    for (i = 0; i < 10; i++) {\n        if (!gc360[i].text[0]',
             b'void fad25()\n{\n    register int i;\n\n    for (i = 0; i < 10; i++) {\n        if (!gc360[i].text[1]'),
            ('F_A09D', b'resource_load_record_into(62, gc360)', b'resource_load_record_into(63, gc360)'),
            ('F_A13F', b'0x3e', b'0x3f')):
            mutant_rejected(name, before, after)

    def test_indexed_base_evidence_controls(self):
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        manifest = read_json(ROOT / 'layout/manifest.json')
        document = read_json(ROOT / 'docs/storage-binding-evidence.json')
        self.assertEqual(verify(document, original, manifest['frames'])['DS_C360'], 0xc360)
        verify_bindings(ROOT, manifest, original)
        for variant in ('base', 'stride', 'target', 'record', 'site', 'extent', 'missing'):
            changed = copy.deepcopy(document)
            item = next(o for o in changed['objects'] if o['id'] == 'DS_C360')
            first = item['observations'][0]
            if variant == 'base': item['offset'] += 1
            elif variant == 'stride': item['record_stride'] += 1
            elif variant == 'target': first['call_target'] += 1
            elif variant == 'record': first['record_id'] += 1
            elif variant == 'site': first['load_offset'] += 1
            elif variant == 'extent': first['function_extent']['end'] -= 1
            else: item['observations'].pop()
            with self.subTest(variant=variant), self.assertRaises(ValueError):
                verify(changed, original, manifest['frames'])
        # Re-pin altered bytes to exercise instruction-form checks after identity checks.
        for index, displacement in ((0, 0), (1, 13)):
            changed = copy.deepcopy(document)
            observation = next(o for o in changed['objects'] if o['id'] == 'DS_C360')['observations'][index]
            modified = bytearray(original)
            modified[512 + observation['load_offset'] + displacement] = 0x0e
            changed['original_sha256'] = hashlib.sha256(modified).hexdigest()
            extent = observation['function_extent']
            extent['sha256'] = hashlib.sha256(modified[extent['start']:extent['end']]).hexdigest()
            with self.assertRaises(ValueError):
                verify(changed, bytes(modified), manifest['frames'])
        changed = copy.deepcopy(manifest)
        next(o for o in changed['regions'] if o['id'] == 'F_68AA')['start'] += 1
        with self.assertRaisesRegex(ValueError, 'callee owner'):
            verify_bindings(ROOT, changed, original)
        changed = copy.deepcopy(manifest)
        next(o for o in changed['regions'] if o['id'] == 'F_A13F')['build']['bindings']['_gc360']['offset'] += 1
        with self.assertRaisesRegex(ValueError, 'Binding contradicts'):
            verify_bindings(ROOT, changed, original)
